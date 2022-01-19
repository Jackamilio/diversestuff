#ifndef __CROPPER_DISPLACER_H__
#define __CROPPER_DISPLACER_H__

#include "Engine.h"
#include <stack>
#include "Rect.h"

class CropperDisplacer : public Engine::Input, public Engine::Graphic {
protected:
	Rect* cropping;
	Rect previousCropping;

public:
	CropperDisplacer();

	void AddForDisplacement(Engine::Input* inputobj, Engine::Graphic* graphicobj);
	void RemoveFromDisplacement(Engine::Input* inputobj, Engine::Graphic* graphicobj);

	bool Event(ALLEGRO_EVENT& event);
	void Draw();
	void PostDraw();

	virtual glm::ivec2 GetDisplaceOffset() const = 0;
};

#endif //__CROPPER_DISPLACER_H__