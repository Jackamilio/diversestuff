#ifndef __DRAGGABLE_H__
#define __DRAGGABLE_H__

#include <glm/glm.hpp>
#include "Engine.h"
#include "DraggableManager.h"

class Draggable : public Engine::Input {
private:
	DraggableManager& manager;
public:
	Draggable(DraggableManager& dgblmgr);

	virtual void Grabbed() {}
	virtual void Dropped() {}
	virtual void Dragged(const glm::ivec2& delta) {}

	virtual bool hitCheck(const glm::ivec2& pos) const = 0;

	bool Event(ALLEGRO_EVENT& ev);
};

#endif //__DRAGGABLE_H__
