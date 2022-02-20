#ifndef __DROP_LOCATION_H__
#define __DROP_LOCATION_H__

#include <glm/glm.hpp>
#include "GuiElement.h"

template<class T>
class Droppable;

class DropLocationBase {
protected:
	DropLocationBase(GuiElement& loc);
public:
    GuiElement& location;

	virtual ~DropLocationBase() {}
};

template<class T>
class DropLocation : public DropLocationBase {
public:
	DropLocation(GuiElement& loc);

    inline void ForceAccept(Droppable<T>* element, GuiElement::Priority priority = GuiElement::Priority::Default);
	inline bool Accept(Droppable<T>* element, const glm::ivec2& pos, GuiElement::Priority priority = GuiElement::Priority::Default);
	inline void Reject(Droppable<T>* element);
};

template<class T>
inline DropLocation<T>::DropLocation(GuiElement& loc) : DropLocationBase(loc)
{
}

template<class T>
inline void DropLocation<T>::ForceAccept(Droppable<T>* element, GuiElement::Priority priority)
{
    if (element->currentDropLocation != this) {
        element->pos -= location.CalculateGlobalOffset();
        location.AddChild(element, priority);
        element->currentDropLocation = this;
    }
}

template<class T>
inline bool DropLocation<T>::Accept(Droppable<T>* element, const glm::ivec2& pos, GuiElement::Priority priority)
{
    if (location.CanAcceptDrop(pos)) {
        ForceAccept(element, priority);
        return true;
    }
    return false;
}

template<class T>
inline void DropLocation<T>::Reject(Droppable<T>* element)
{
    if (element->currentDropLocation == this) {
        element->pos += location.CalculateGlobalOffset();
        location.RemoveChild(element);
        element->currentDropLocation = nullptr;
    }
}

#endif //__DROP_LOCATION_H__