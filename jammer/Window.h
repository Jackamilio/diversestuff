#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Engine.h"
#include "Draggable.h"
#include "Rect.h"
#include "CropperDisplacer.h"

class Window : public Rect, public virtual GuiElement, virtual public Draggable, virtual public CropperDisplacer {
private:
	int headBandHeight;
public:
	Window();
	~Window();

	Rect HeadBandRect() const;

	virtual void Draw();
	virtual void PostDraw();
	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);

	virtual void Grabbed();
	virtual void Dragged(const glm::ivec2& delta);
	virtual bool hitCheck(const glm::ivec2& pos) const;
	virtual void SetPos(const glm::ivec2& tsl);
	virtual glm::ivec2 GetPos() const;

	virtual glm::ivec2 GetDisplaceOffset() const;
};

#endif //__WINDOW_H__