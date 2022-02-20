#ifndef __DRAGGABLE_H__
#define __DRAGGABLE_H__

#include <glm/glm.hpp>
#include "Engine.h"
#include "GuiElement.h"

class DropLocationBase;

class Draggable : virtual public GuiElement {
public:
	struct GrabProperties {
		glm::ivec2 position;
		DropLocationBase* location;
		GuiElement::Priority priority;
	};

	virtual ~Draggable() {}

	virtual void Grabbed() {}
	virtual void Dropped() {}
	virtual void Dragged(const glm::ivec2& delta) { pos += delta; }

	virtual void CancelGrab();

	virtual bool hitCheck(const glm::ivec2& pos) const = 0;

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& ev);

	void ForceGrab();
};

#endif //__DRAGGABLE_H__
