#ifndef __DRAGGABLE_H__
#define __DRAGGABLE_H__

#include <glm/glm.hpp>
#include "Engine.h"
#include "GuiElement.h"

class Draggable : virtual public GuiElement {
public:
	virtual ~Draggable() {}

	virtual void Grabbed() {}
	virtual void Dropped() {}
	virtual void Dragged(const glm::ivec2& delta) {}

	virtual void SetPos(const glm::ivec2& tsl) = 0;
	virtual glm::ivec2 GetPos() const = 0;
	inline void Move(const glm::ivec2& tsl) { SetPos(GetPos() + tsl); }
	virtual bool hitCheck(const glm::ivec2& pos) const = 0;

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& ev);

	void ForceGrab();
};

#endif //__DRAGGABLE_H__
