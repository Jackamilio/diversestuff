#include "Button.h"
#include "DefaultColors.h"

Button::Button() : state(Button::State::Neutral)
{
}

Button::~Button()
{
}

bool Button::Event(ALLEGRO_EVENT& event)
{
    if ( (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) && event.mouse.button == 1
        || event.type == ALLEGRO_EVENT_MOUSE_AXES ) {
        if (isInside(MousePosition(event))) {
            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                state = State::Clicked;
                return true;
            }
            else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
                Fire(EventType::Clicked);
                state = State::Hovered;
            }
            else if (state == State::NeutralHeld) {
                state = State::Clicked;
            }
            else if (state == State::Neutral) {
                state = State::Hovered;
            }
        }
        else {
            if (state == State::Clicked) {
                state = State::NeutralHeld;
            }
            else if (state == State::Hovered || event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
                state = State::Neutral;
            }
        }
    }
    return false;
}

void Button::Draw()
{
	draw_filled(state == State::Clicked ? lightgrey : (state == State::Hovered ? darkgrey : black));
	draw(white, 1);
}
