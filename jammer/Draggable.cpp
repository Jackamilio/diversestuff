#include "Draggable.h"

Engine::InputStatus Draggable::Event(ALLEGRO_EVENT& event) {
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (hitCheck(glm::ivec2(event.mouse.x, event.mouse.y))) {
            Grabbed();
            gui.Track(this);
            return Engine::InputStatus::grabbed;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if (gui.IsTracked(this)) {
            Dropped();
            gui.UnTrack(this);
            return Engine::InputStatus::grabbed;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && gui.IsTracked(this)) {
        Dragged(glm::ivec2(event.mouse.dx, event.mouse.dy));

        return Engine::InputStatus::grabbed;
    }
    return Engine::InputStatus::ignored;
}