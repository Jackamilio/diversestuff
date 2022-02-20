#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Engine.h"
#include "Draggable.h"
#include "Rect.h"
#include "Cropper.h"

#pragma warning( push )
#pragma warning( disable : 4250 )
class Window : public Rect, virtual public Draggable, virtual public Cropper {
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
	virtual bool Event(ALLEGRO_EVENT& event);

	virtual void Grabbed();
	virtual bool hitCheck(const glm::ivec2& pos) const;
};
#pragma warning( pop )

#endif //__WINDOW_H__