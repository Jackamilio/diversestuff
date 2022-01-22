#include "Button.h"
#include "DefaultColors.h"

Button::Button() : state(Button::State::Neutral)
{
}

Button::~Button()
{
}

Engine::InputStatus Button::Event(ALLEGRO_EVENT& event)
{
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || event.type == ALLEGRO_EVENT_MOUSE_AXES || event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if (isInside(glm::ivec2(event.mouse.x, event.mouse.y))) {
            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                state = State::Clicked;
                return Engine::InputStatus::grabbed;
            }
            else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
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
    return Engine::InputStatus::ignored;
}

void Button::Draw()
{
	draw_filled(state == State::Clicked ? lightgrey : (state == State::Hovered ? darkgrey : black));
	draw(white, 1);
}
