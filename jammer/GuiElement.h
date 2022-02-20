#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

#include <vector>
#include <map>
#include <functional>
#include "Engine.h"

class GuiMaster;
class Cropper;

class GuiElement;

class GuiElement {
public:
	enum class EventType { Opened, Closed, Clicked, Moved, Resized };
	typedef std::function<void()> EventReaction;
	std::map<EventType, std::map<void*, EventReaction>> eventReactions;

	enum class Priority : int {
		Unknown = 0,
		Bottom,
		Default,
		Top,
	};

	typedef std::vector<GuiElement*> GuiElementVector;
	typedef std::map<Priority, GuiElementVector> ChildList;
private:
	GuiElement* parent;
	ChildList children;
//protected:
//	bool eventBeforeChildren;

public:
	GuiMaster& gui;
	glm::ivec2 pos;

	template<typename Elem, class Map, class MapIt>
	class TemplateIterator {
		friend class GuiElement;
	private:
		MapIt mapIt, mapEnd;
		int curElem;

		TemplateIterator(MapIt mi, MapIt me);
		TemplateIterator(MapIt mi, MapIt me, int e);
	public:

		bool operator !=(const TemplateIterator& r) const;
		void operator ++();
		Elem operator*();
	};

	typedef TemplateIterator<GuiElement*, ChildList, ChildList::iterator> Iterator;
	typedef TemplateIterator<GuiElement*, ChildList, ChildList::reverse_iterator> ReverseIterator;
	typedef TemplateIterator<const GuiElement*, const ChildList, ChildList::const_iterator> ConstIterator;

	Iterator begin();
	Iterator end();
	ReverseIterator rbegin();
	ReverseIterator rend();
	ConstIterator cbegin() const;
	ConstIterator cend() const;
	inline ConstIterator begin() const { return cbegin(); }
	inline ConstIterator end() const { return cend(); }
	Iterator find(GuiElement* c);
	ConstIterator find(const GuiElement* c) const;

	void AddChild(GuiElement* c, Priority p = Priority::Default);
	void RemoveChild(GuiElement* c);

	inline GuiElement* Parent() { return parent; }
	inline const GuiElement* Parent() const { return parent; }

	Priority FindMyPriority(Priority valIfNotFound = Priority::Unknown) const;
	//inline bool IsEventBeforeChildren() const { return eventBeforeChildren; }

	GuiElement();
	virtual ~GuiElement();

	void PutOnTop();
	void PutAtBottom();

	glm::ivec2 CalculateGlobalOffset() const;

	//int CalculatePriority() const;
	//typedef std::vector<const GuiElement*> Lineage;
	class Lineage : public std::vector<const GuiElement*> {
	public:
		bool operator<(const Lineage& rhs) const;
	};
	Lineage CompileLineage() const;

	virtual bool Event(ALLEGRO_EVENT& event) { return false; }
	virtual void Draw() {}
	virtual void PostDraw() {}

	// event system
	void ReactTo(EventType evtype, void* subscriberID, const EventReaction& reaction);
	void CancelReaction(EventType evtype, void* subscriberID);
	void Fire(EventType ev);
};

template<class Elem, class Map, class MapIt>
inline GuiElement::TemplateIterator<Elem, Map, MapIt>::TemplateIterator(MapIt mi, MapIt me) : mapIt(mi), mapEnd(me), curElem((mapIt == mapEnd) ? -1 : 0)
{
}

template<class Elem, class Map, class MapIt>
inline GuiElement::TemplateIterator<Elem, Map, MapIt>::TemplateIterator(MapIt mi, MapIt me, int e) : mapIt(mi), mapEnd(me), curElem(e)
{
}

template<class Elem, class Map, class MapIt>
inline bool GuiElement::TemplateIterator<Elem, Map, MapIt>::operator!=(const TemplateIterator& r) const
{
	return mapIt != r.mapIt || curElem != r.curElem;
}

template<class Elem, class Map, class MapIt>
inline void GuiElement::TemplateIterator<Elem, Map, MapIt>::operator++()
{
	if (++curElem >= (int)mapIt->second.size()) {
		curElem = (++mapIt == mapEnd) ? -1 : 0;
	}
}

template<class Elem, class Map, class MapIt>
inline Elem GuiElement::TemplateIterator<Elem, Map, MapIt>::operator*()
{
	return mapIt->second[curElem];
}

template<>
inline GuiElement* GuiElement::TemplateIterator<GuiElement*, GuiElement::ChildList, GuiElement::ChildList::reverse_iterator>::operator*()
{
	return mapIt->second[(int)mapIt->second.size() - curElem - 1];
}

#endif//__GUI_ELEMENT_H__
