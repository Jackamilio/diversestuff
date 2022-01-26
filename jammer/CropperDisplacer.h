#ifndef __CROPPER_DISPLACER_H__
#define __CROPPER_DISPLACER_H__

#include <stack>
#include "Rect.h"
#include "GuiElement.h"

class CropperDisplacer : virtual public GuiElement {
protected:
	Rect* cropping;
	Rect previousCropping;

public:
	CropperDisplacer();

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);
	virtual void Draw();
	virtual void PostDraw();

	virtual glm::ivec2 GetDisplaceOffset() const = 0;
	glm::ivec2 CalculateGlobalDisplaceOffset() const;

	inline bool InsideCropping(const glm::ivec2& pos) const {
		if (cropping) {
			return cropping->isInside(pos);
		}
		return true;
	}
};

#endif //__CROPPER_DISPLACER_H__