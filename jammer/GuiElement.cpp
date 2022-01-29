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
	if (parent) {
		parent->RemoveChild(this);
	}
	for (int i = 0; i < ChildrenSize(); ++i) {
		GetChild(i)->parent = nullptr;
	}
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

/*int GuiElement::CalculatePriority() const
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
}*/

bool GuiElement::Lineage::operator<(const Lineage& rhs) const
{
	Lineage::const_reverse_iterator lcri = crbegin();
	Lineage::const_reverse_iterator rcri = rhs.crbegin();

	Lineage::const_reverse_iterator end_lcri = crend();
	Lineage::const_reverse_iterator endrcri = rhs.crend();

	if (lcri != end_lcri && rcri != endrcri) {
		Lineage::const_reverse_iterator prev_lcri = lcri++;
		Lineage::const_reverse_iterator prev_rcri = rcri++;

		while (lcri != end_lcri && rcri != endrcri) {
			if (*lcri != *rcri) { // we are finally different
				const GuiElement* sameParent = *prev_lcri;
				// look for oursleves
				for (int i = 0; i < sameParent->ChildrenSize(); ++i) {
					const GuiElement* curChild = sameParent->GetChild(i);
					if (curChild == *lcri) {
						return true; // self found first, we are bigger!
					}
					else if (curChild == *rcri) {
						return false; // rhs found first, we are smaller!
					}
				}
				// same parent found but neither child was found? logically code should never reach here
				return false;
			}

			// 4 increments wow
			++lcri;
			++rcri;
			++prev_lcri;
			++prev_rcri;
		}
	}
	// in this case, either we are both from totally different lineage (at least one of us wasn't added to the gui master)
	// or we are comparing to gui master itself, and we're bigger if we're still valid
	return lcri != end_lcri;
}

GuiElement::Lineage GuiElement::CompileLineage() const
{
	Lineage ret;

	const GuiElement* current = this;
	while (current) {
		ret.push_back(current);
		current = current->Parent();
	}

	return ret;
}
