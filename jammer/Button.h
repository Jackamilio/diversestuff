#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Rect.h"
#include "GuiMaster.h"

class Button : public Rect, public GuiElement {
public:
	enum class State {
		Neutral,
		Hovered,
		Clicked,
		NeutralHeld
	};

private:

	State state;

public:

	Button();
	~Button();

	virtual bool Event(ALLEGRO_EVENT& event);
	virtual void Draw();
};

#endif //__BUTTON_H__