#ifndef __CROPPER_H__
#define __CROPPER_H__

#include <stack>
#include "Rect.h"
#include "GuiElement.h"

class Cropper : virtual public GuiElement {
protected:
	const Rect& cropping;
	Rect previousCropping;

public:
	Cropper(const Rect& cropref);

	virtual void Draw();
	virtual void PostDraw();

	inline bool InsideCropping(const glm::ivec2& pos) const { return cropping.isInside(pos); }
};

#endif //__CROPPER_DISPLACER_H__