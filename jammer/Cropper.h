#ifndef __CROPPER_H__
#define __CROPPER_H__

#include "Rect.h"
#include "GuiElement.h"

class Cropper : public virtual GuiElement {
protected:
	const Rect& cropping;
	Rect previousCropping;

public:
	Cropper(const Rect& cropref);

	virtual void Draw();
	virtual void PostDraw();
	virtual bool Event(ALLEGRO_EVENT& event);

	virtual bool CanAcceptDrop(const glm::ivec2& atpos) const;
};

#endif //__CROPPER_H__