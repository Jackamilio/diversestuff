#ifndef __CROPPER_DISPLACER_H__
#define __CROPPER_DISPLACER_H__

#include <stack>
#include "Rect.h"
#include "GuiMaster.h"

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
	glm::ivec2 CalculateGlobalDIsplaceOffset() const;

	inline bool InsideCropping(const glm::ivec2& pos) const {
		return cropping && cropping->isInside(pos);
	}
};

#endif //__CROPPER_DISPLACER_H__