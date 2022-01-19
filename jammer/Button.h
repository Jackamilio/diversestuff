#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Rect.h"
#include "Engine.h"

class Button : public Rect, public Engine::Input, public Engine::Graphic {
public:
	enum class State {
		Neutral,
		Hovered,
		Clicked,
		NeutralHeld
	};

	OTN(Button);
private:

	State state;

public:

	Button();
	~Button();

	bool Event(ALLEGRO_EVENT& event);
	void Draw();
};

#endif //__BUTTON_H__