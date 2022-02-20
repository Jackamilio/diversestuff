#ifndef __PURE_DISPLACER_H__
#define __PURE_DISPLACER_H__

#include "Draggable.h"
#include "Cropper.h"

#pragma warning( push )
#pragma warning( disable : 4250 )
/*class PureDisplacer : virtual public GuiElement, public CropperDisplacer, public Draggable {
public:
	PureDisplacer();

	virtual bool Event(ALLEGRO_EVENT& event);
	virtual bool hitCheck(const glm::ivec2& pos) const;
};*/
class PureDisplacer : public Cropper, public Draggable {
public:
	virtual bool hitCheck(const glm::ivec2& pos) const;
};
#pragma warning( pop )

#endif //__PURE_DISPLACER_H__