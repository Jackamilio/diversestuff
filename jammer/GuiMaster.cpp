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

GuiMaster::GuiMaster() : trackedDraggable(nullptr)
{
	InitTransforms();
}

Engine::InputStatus GuiMaster::Event(ALLEGRO_EVENT& event)
{
	return RecursiveEvent(this, event, false) ? Engine::InputStatus::grabbed : Engine::InputStatus::ignored;
}

void GuiMaster::Draw()
{
	RecursiveDraw(this, false);
}

bool GuiMaster::RecursiveEvent(GuiElement* guielem, ALLEGRO_EVENT& event, bool doroot)
{
	Engine::InputStatus ret = Engine::InputStatus::ignored;
	if (doroot) {
		ret = guielem->Event(event);
	}

	if (ret == Engine::InputStatus::ignored) {
		for (int i = 0; i < guielem->ChildrenSize(); ++i) {
			if (RecursiveEvent(guielem->GetChild(i), event)) {
				return true;
			}
		}
	}
	return ret == Engine::InputStatus::grabbed;
}

void GuiMaster::RecursiveDraw(GuiElement* guielem, bool doroot)
{
	if (doroot) {
		guielem->Draw();
	}

	for (int i = guielem->ChildrenSize() - 1; i >= 0 ; --i) {
		RecursiveDraw(guielem->GetChild(i));
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

void GuiElement::ReplaceParentFromChild(GuiElement* child)
{
	if (child->parent && parent != this) {
		child->parent->RemoveChild(child);
	}
	child->parent = this;
}

GuiElement::GuiElement() : parent(nullptr), gui(GuiMaster::Get())
{
}

GuiElement::~GuiElement()
{
}

void GuiElement::PutOnTop()
{
	if (Parent()) {
		Parent()->AddChild(this, true);
	}
}

void GuiElement::PutAtBottom()
{
	if (Parent()) {
		Parent()->AddChild(this, false);
	}
}
