#ifndef __PURE_DISPLACER_H__
#define __PURE_DISPLACER_H__

#include "CropperDisplacer.h"
#include "Draggable.h"

#pragma warning( push )
#pragma warning( disable : 4250 )
class PureDisplacer : virtual public GuiElement, public CropperDisplacer, public Draggable {
public:
	glm::ivec2 offset;

	PureDisplacer();

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);
	virtual glm::ivec2 GetDisplaceOffset() const;
	virtual void SetPos(const glm::ivec2& tsl);
	virtual glm::ivec2 GetPos() const;
	virtual void Dragged(const glm::ivec2& delta);
	virtual bool hitCheck(const glm::ivec2& pos) const;
};
#pragma warning( pop )

#endif //__PURE_DISPLACER_H__