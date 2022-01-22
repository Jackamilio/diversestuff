#ifndef __DRAGGABLE_H__
#define __DRAGGABLE_H__

#include <glm/glm.hpp>
#include "Engine.h"
#include "GuiMaster.h"

class Draggable : virtual public GuiElement {
public:
	virtual void Grabbed() {}
	virtual void Dropped() {}
	virtual void Dragged(const glm::ivec2& delta) {}

	virtual bool hitCheck(const glm::ivec2& pos) const = 0;
	virtual Engine::InputStatus Event(ALLEGRO_EVENT& ev);
};

#endif //__DRAGGABLE_H__
