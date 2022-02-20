#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Engine.h"
#include "Draggable.h"
#include "Rect.h"
#include "CropperDisplacer.h"

class Window : public Rect, public virtual GuiElement, virtual public Draggable, virtual public CropperDisplacer {
private:
	int headBandHeight;

	int* horiResize;
	int* vertResize;
	int trackedHoriResize;
	int trackedVertResize;
public:
	int minWidth;
	int minHeight;

	Window();
	~Window();

	Rect HeadBandRect() const;

	virtual void Draw();
	virtual void PostDraw();
	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);

	virtual void Grabbed();
	virtual bool hitCheck(const glm::ivec2& pos) const;

	virtual glm::ivec2 GetDisplaceOffset() const;
};

#endif //__WINDOW_H__