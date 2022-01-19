#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Engine.h"
#include "Draggable.h"
#include "DraggableManager.h"
#include "Rect.h"
#include "CropperDisplacer.h"

class Window : public Draggable, public Rect, public CropperDisplacer, public Engine::Input {
private:
	int headBandHeight;
public:
	OTN(Window);

	Window(DraggableManager& manager);
	~Window();

	Rect HeadBandRect() const;

	void Draw();
	void PostDraw();
	bool Event(ALLEGRO_EVENT& event);

	virtual void Dragged(const glm::ivec2& delta);
	virtual bool hitCheck(const glm::ivec2& pos) const;

	virtual glm::ivec2 GetDisplaceOffset() const;
};

#endif //__WINDOW_H__