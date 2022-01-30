#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

#include <vector>
#include <map>
#include "Engine.h"

class GuiMaster;
class CropperDisplacer;

class GuiElement;

class GuiElement {
public:
	enum class Priority : int {
		Top = 1,
		Default,
		Bottom
	};

private:
	GuiElement* parent;

	typedef std::vector<GuiElement*> GuiElementVector;
	typedef std::map<Priority, GuiElementVector> ChildList;
	ChildList children;

public:
	GuiMaster& gui;

	template<class It, class T>
	using beginorend = It(T::*)();

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
	ConstIterator begin() const;
	ConstIterator end() const;
	Iterator find(GuiElement* c);

	void AddChild(GuiElement* c, Priority p = Priority::Default);
	void RemoveChild(GuiElement* c);

	inline GuiElement* Parent() { return parent; }
	inline const GuiElement* Parent() const { return parent; }

	GuiElement();
	virtual ~GuiElement();

	void PutOnTop();
	void PutAtBottom();

	//int CalculatePriority() const;
	//typedef std::vector<const GuiElement*> Lineage;
	class Lineage : public std::vector<const GuiElement*> {
	public:
		bool operator<(const Lineage& rhs) const;
	};
	Lineage CompileLineage() const;

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event) { return Engine::InputStatus::ignored; }
	virtual void Draw() {}
	virtual void PostDraw() {}
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

#endif//__GUI_ELEMENT_H__
