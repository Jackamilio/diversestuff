#ifndef __DROP_LOCATION_H__
#define __DROP_LOCATION_H__

#include <glm/glm.hpp>
#include "Draggable.h"
#include "CropperDisplacer.h"

template<class T>
class Droppable;

class DropLocationBase {
protected:
	DropLocationBase(CropperDisplacer& loc);
public:
    CropperDisplacer& location;

    inline glm::ivec2 GetGlobalOffset() const { return location.CalculateGlobalDisplaceOffset(); }

	virtual ~DropLocationBase() {}
};

template<class T>
class DropLocation : public DropLocationBase {
public:
	DropLocation(CropperDisplacer& loc);

    inline void ForceAccept(Droppable<T>* element, GuiElement::Priority priority = GuiElement::Priority::Default);
	inline bool Accept(Droppable<T>* element, const glm::ivec2& pos, GuiElement::Priority priority = GuiElement::Priority::Default);
	inline void Reject(Droppable<T>* element);
};

template<class T>
inline DropLocation<T>::DropLocation(CropperDisplacer& loc) : DropLocationBase(loc)
{
}

template<class T>
inline void DropLocation<T>::ForceAccept(Droppable<T>* element, GuiElement::Priority priority)
{
    if (element->currentDropLocation != this) {
        element->Move(-GetGlobalOffset());
        location.AddChild(element, priority);
        element->currentDropLocation = this;
    }
}

template<class T>
inline bool DropLocation<T>::Accept(Droppable<T>* element, const glm::ivec2& pos, GuiElement::Priority priority)
{
    glm::ivec2 globaloffset(GetGlobalOffset());
    if (location.InsideCropping(pos - globaloffset + location.GetDisplaceOffset())) {
        if (element->currentDropLocation != this) {
            element->Move(-globaloffset);
            location.AddChild(element, priority);
            element->currentDropLocation = this;
        }
        return true;
    }
    return false;
}

template<class T>
inline void DropLocation<T>::Reject(Droppable<T>* element)
{
    if (element->currentDropLocation == this) {
        element->Move(GetGlobalOffset());
        location.RemoveChild(element);
        element->currentDropLocation = nullptr;
    }
}

#endif //__DROP_LOCATION_H__