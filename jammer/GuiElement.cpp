#include "GuiElement.h"
#include "GuiMaster.h"

GuiElement::Iterator GuiElement::begin()
{
	return Iterator(children.begin(), children.end());
}

GuiElement::Iterator GuiElement::end()
{
	return Iterator(children.end(), children.end());
}

GuiElement::ReverseIterator GuiElement::rbegin()
{
	return ReverseIterator(children.rbegin(), children.rend());
}

GuiElement::ReverseIterator GuiElement::rend()
{
	return ReverseIterator(children.rend(), children.rend());
}

GuiElement::ConstIterator GuiElement::cbegin() const
{
	return ConstIterator(children.begin(), children.end());
}

GuiElement::ConstIterator GuiElement::cend() const
{
	return ConstIterator(children.end(), children.end());
}

GuiElement::Iterator GuiElement::find(GuiElement* c)
{
	// WARNING : THIS IS COPIED BELOW!! UPDATE AS NEEDED!!!
	for (auto mapIt = children.begin(); mapIt != children.end(); ++mapIt) {
		std::vector<GuiElement*>& vec = mapIt->second;
		for (int i = 0; i < (int)vec.size(); ++i) {
			if (vec[i] == c) {
				return Iterator(mapIt, children.end(), i);
			}
		}
	}
	return end();
}

GuiElement::ConstIterator GuiElement::find(const GuiElement* c) const
{
	// WARNING : COPY PASTE OF ABOVE!!! UPDATE AS NEEDED!!!
	for (auto mapIt = children.begin(); mapIt != children.end(); ++mapIt) {
		const std::vector<GuiElement*>& vec = mapIt->second;
		for (int i = 0; i < (int)vec.size(); ++i) {
			if (vec[i] == c) {
				return ConstIterator(mapIt, children.end(), i);
			}
		}
	}
	return cend();
}

void GuiElement::AddChild(GuiElement* c, Priority p)
{
	// replace parent
	if (c->parent) {
		if (parent == this) {
			return;
		}
		else {
			c->parent->RemoveChild(c);
		}
	}
	c->parent = this;

	children[p].push_back(c);
}

void GuiElement::RemoveChild(GuiElement* c)
{
	c->parent = nullptr;
	/*for (auto vecs : children) {
		std::vector<GuiElement*>& vec = vecs.second;
		for (auto it = vec.begin(); it != vec.end(); ++it) {
			if (*it == c) {
				vec.erase(it);
				return;
			}
		}
	}*/
	Iterator it = find(c);
	if (it != end()) {
		it.mapIt->second.erase(it.mapIt->second.begin()+it.curElem);
		if (it.mapIt->second.empty()) {
			children.erase(it.mapIt);
		}
	}
}

GuiElement::Priority GuiElement::FindMyPriority(GuiElement::Priority valIfNotFound) const
{
	if (parent) {
		ConstIterator it = parent->find(this);
		if (it != parent->cend()) {
			return it.mapIt->first;
		}
	}
	return valIfNotFound;
}

GuiElement::GuiElement() : parent(nullptr), gui(GuiMaster::Get()), pos{}
{
	children.clear();
}

GuiElement::~GuiElement()
{
	if (parent) {
		parent->RemoveChild(this);
	}
	for (auto child : *this) {
		child->parent = nullptr;
	}
}

void GuiElement::PutOnTop()
{
	if (parent) {
		Iterator it = parent->find(this);
		if (it != parent->end()) {
			it.mapIt->second.erase(it.mapIt->second.begin() + it.curElem);
			it.mapIt->second.push_back(this);
		}
	}
}

void GuiElement::PutAtBottom()
{
	if (parent) {
		Iterator it = parent->find(this);
		if (it != parent->end()) {
			it.mapIt->second.erase(it.mapIt->second.begin() + it.curElem);
			it.mapIt->second.insert(it.mapIt->second.begin(), this);
		}
	}
}

glm::ivec2 GuiElement::CalculateGlobalOffset() const
{
	glm::ivec2 offset = pos;

	const GuiElement* parent = Parent();
	while (parent)
	{
		const Cropper* cdparent = dynamic_cast<const Cropper*>(parent);
		if (cdparent) {
			offset += cdparent->pos;
		}
		parent = parent->Parent();
	}

	return offset;
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
				for (auto curChild : *sameParent) {
					if (curChild == *lcri) {
						return false; // self found first, we are smaller!
					}
					else if (curChild == *rcri) {
						return true; // rhs found first, we are bigger!
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

void GuiElement::ReactTo(EventType evtype, void* subscriberID, const EventReaction& reaction)
{
	eventReactions[evtype][subscriberID] = reaction;
}

void GuiElement::CancelReaction(EventType evtype, void* subscriberID)
{
	auto typemap = eventReactions.find(evtype);
	if (typemap != eventReactions.end()) {
		auto reaction = typemap->second.find(subscriberID);
		if (reaction != typemap->second.end()) {
			typemap->second.erase(reaction);
		}
		if (typemap->second.empty()) {
			eventReactions.erase(typemap);
		}
	}
}

void GuiElement::Fire(EventType ev)
{
	auto typemap = eventReactions.find(ev);
	if (typemap != eventReactions.end()) {
		for (auto it : typemap->second) {
			it.second();
		}
	}
}
