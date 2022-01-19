#include "Draggable.h"

Draggable::Draggable(DraggableManager& dgblmgr) : manager(dgblmgr)
{
}

bool Draggable::Event(ALLEGRO_EVENT& event) {
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (hitCheck(glm::ivec2(event.mouse.x, event.mouse.y))) {
            Grabbed();
            manager.Track(this);
            return true;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if (manager.IsTracked(this)) {
            Dropped();
            manager.UnTrack(this);
            return true;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && manager.IsTracked(this)) {
        Dragged(glm::ivec2(event.mouse.dx, event.mouse.dy));

        return true;
    }
    return false;
}