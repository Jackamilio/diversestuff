#include "GuiMaster.h"
#include "Draggable.h"
#include <allegro5/allegro_windows.h>

void GuiMaster::InitTransforms()
{
	std::stack<ALLEGRO_TRANSFORM> ot;
	transforms.swap(ot);
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	transforms.push(t);
}

GuiMaster* singleton = nullptr;

void GuiMaster::Init()
{
	if (!singleton) { singleton = new GuiMaster; }
}

void GuiMaster::End()
{
	delete singleton;
	singleton = nullptr;
}

GuiMaster& GuiMaster::Get()
{
	return *singleton;
}

struct WindowsCursorCallbackData {
	ALLEGRO_DISPLAY* display;
	ALLEGRO_SYSTEM_MOUSE_CURSOR cursor;
};

WindowsCursorCallbackData wccd;

bool WindowsCursorCallback(ALLEGRO_DISPLAY* display, UINT message, WPARAM wparam,
	LPARAM lparam, LRESULT* result, void* userdata) {
		if (message == WM_SETCURSOR) {
			al_set_system_mouse_cursor(wccd.display, wccd.cursor);
			return true;
		}
		return false;
}

GuiMaster::GuiMaster() : trackedDraggable(nullptr), focus(nullptr), caretTimer(nullptr), caretVisible(true), numericalChars(nullptr)
{
	InitTransforms();
	caretTimer = al_create_timer(0.5);
	al_register_event_source(engine.eventQueue, al_get_timer_event_source(caretTimer));
	numericalChars = al_ustr_new("0123456789.");

	// platform specific workaround for windows to be able to change the f***** cursor
	// this needs to be ported (or ifdef'd out if everything works as intended) for other platforms
	al_win_add_window_callback(engine.display, WindowsCursorCallback, nullptr);
	wccd.display = engine.display;
	RequestCursor(nullptr, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
}

GuiMaster::~GuiMaster()
{
	al_win_remove_window_callback(engine.display, WindowsCursorCallback, nullptr);
	al_ustr_free(numericalChars);
	al_destroy_timer(caretTimer);
	for (auto dlv : dropLocations) {
		for (auto dl : dlv.second) {
			delete dl;
		}
	}
}

static bool isMouseEvent = false;

bool GuiMaster::Event(ALLEGRO_EVENT& event)
{
	if (event.type == ALLEGRO_EVENT_TIMER && event.any.source == al_get_timer_event_source(caretTimer)) {
		caretVisible = !caretVisible;
		return true;
	}
	isMouseEvent = IsMouseEvent(event);
	return RecursiveEvent(this, event, false);
}

void GuiMaster::Draw()
{
	RecursiveDraw(this, false);
}

bool GuiMaster::RecursiveEvent(GuiElement* guielem, ALLEGRO_EVENT& event, bool doroot)
{
	if (isMouseEvent) {
		event.mouse.x -= guielem->pos.x;
		event.mouse.y -= guielem->pos.y;
	}

	for (auto it = guielem->rbegin(); it != guielem->rend(); ++it) {
		if (RecursiveEvent(*it, event)) {
			return true;
		}
	}

	bool ret = false;
	if (doroot) ret = guielem->Event(event);

	if (isMouseEvent) {
		event.mouse.x += guielem->pos.x;
		event.mouse.y += guielem->pos.y;
	}

	return ret;
}

void GuiMaster::RecursiveDraw(GuiElement* guielem, bool doroot)
{
	PushTransform();
	TranslateTransform(guielem->pos);
	if (doroot) {
		guielem->Draw();
	}

	for (auto it = guielem->begin(); it != guielem->end(); ++it) {
		RecursiveDraw(*it);
	}

	if (doroot) {
		guielem->PostDraw();
	}
	PopTransform();
}

void GuiMaster::Track(Draggable* dgbl)
{
	if (trackedDraggable && trackedDraggable != dgbl) {
		trackedDraggable->Dropped();
	}

	trackedDraggable = dgbl;
}

void GuiMaster::UnTrack(Draggable* dgbl)
{
	if (trackedDraggable == dgbl) {
		trackedDraggable = nullptr;
	}
}

bool GuiMaster::IsTracked(Draggable* dgbl) const
{
	return trackedDraggable == dgbl;
}

void GuiMaster::RequestFocus(GuiElement* fe)
{
	focus = fe;
}

void GuiMaster::CancelFocus(GuiElement* fe)
{
	if (HasFocus(fe)) {
		focus = nullptr;
	}
}

bool GuiMaster::HasFocus(GuiElement* fe) const
{
	return focus == fe;
}

bool GuiMaster::IsCaretVisible() const
{
	return caretVisible;
}

void GuiMaster::ResetCaret()
{
	al_stop_timer(caretTimer);
	al_start_timer(caretTimer);
	caretVisible = true;
}

void GuiMaster::PushTransform()
{
	transforms.push(*al_get_current_transform());
}

void GuiMaster::PopTransform()
{
	transforms.pop();
	al_use_transform(&CurrentTransform());
}

void GuiMaster::IdentityTransform()
{
	al_identity_transform(&CurrentTransform());
	al_use_transform(&CurrentTransform());
}

void GuiMaster::TranslateTransform(const glm::ivec2& offset)
{
	al_translate_transform(&CurrentTransform(), offset.x, offset.y);
	al_use_transform(&CurrentTransform());
}

void GuiMaster::RequestCursor(const GuiElement* requester, ALLEGRO_SYSTEM_MOUSE_CURSOR cursor_id)
{
	requestedCursors[requester] = cursor_id;

	// platform specific stuff, see the constructor
	wccd.cursor = cursor_id;
}

void GuiMaster::CancelCursor(const GuiElement* requester)
{
	auto it = requestedCursors.find(requester);
	if (it != requestedCursors.end()) {
		requestedCursors.erase(it);
		wccd.cursor = requestedCursors.rbegin()->second;
	}
}

