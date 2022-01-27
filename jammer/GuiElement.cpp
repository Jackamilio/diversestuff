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

int GuiElement::CalculatePriority() const
{
	int parentpriority = 0;
	const GuiElement* parent = Parent();

	int siblingpriority = 0;
	if (parent) {
		for (; siblingpriority < parent->ChildrenSize(); ++siblingpriority) {
			if (parent->GetChild(siblingpriority) == this) {
				siblingpriority = parent->ChildrenSize() - siblingpriority;
				break;
			}
		}
	}

	while (parent) {
		++parentpriority;
		parent = parent->Parent();
	}

	return parentpriority << 16 | siblingpriority;
}
