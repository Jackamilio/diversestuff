#include "GuiMaster.h"
#include "Draggable.h"

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

GuiMaster::GuiMaster() : trackedDraggable(nullptr), focus(nullptr), caretTimer(nullptr), caretVisible(true)
{
	InitTransforms();
	caretTimer = al_create_timer(0.5);
	al_register_event_source(engine.eventQueue, al_get_timer_event_source(caretTimer));
}

GuiMaster::~GuiMaster()
{
	al_destroy_timer(caretTimer);
	for (auto dlv : dropLocations) {
		for (auto dl : dlv.second) {
			delete dl;
		}
	}
}

Engine::InputStatus GuiMaster::Event(ALLEGRO_EVENT& event)
{
	if (event.type == ALLEGRO_EVENT_TIMER && event.any.source == al_get_timer_event_source(caretTimer)) {
		caretVisible = !caretVisible;
		return Engine::InputStatus::grabbed;
	}
	return RecursiveEvent(this, event, false) ? Engine::InputStatus::grabbed : Engine::InputStatus::ignored;
}

void GuiMaster::Draw()
{
	RecursiveDraw(this, false);
}

glm::ivec2 GuiMaster::GetDisplaceOffset() const
{
	return glm::ivec2();
}

bool GuiMaster::RecursiveEvent(GuiElement* guielem, ALLEGRO_EVENT& event, bool doroot)
{
	Engine::InputStatus ret = Engine::InputStatus::ignored;
	if (doroot && guielem->EventBeforeChildren()) {
		ret = guielem->Event(event);
	}

	if (ret == Engine::InputStatus::ignored) {
		for (auto it = guielem->rbegin(); it != guielem->rend(); ++it) {
			if (RecursiveEvent(*it, event)) {
				return true;
			}
		}
	}

	if (doroot && !guielem->EventBeforeChildren()) {
		ret = guielem->Event(event);
	}

	return ret == Engine::InputStatus::grabbed;
}

void GuiMaster::RecursiveDraw(GuiElement* guielem, bool doroot)
{
	if (doroot) {
		guielem->Draw();
	}

	for (auto it = guielem->begin(); it != guielem->end(); ++it) {
		RecursiveDraw(*it);
	}

	if (doroot) {
		guielem->PostDraw();
	}
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
	al_stop_timer(caretTimer);
	al_start_timer(caretTimer);
	caretVisible = true;
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
