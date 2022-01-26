#include "GuiElement.h"
#include "GuiMaster.h"

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