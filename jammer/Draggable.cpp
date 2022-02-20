#include "Draggable.h"
#include "Cropper.h"
#include "GuiMaster.h"

void Draggable::CancelGrab()
{
    gui.UnTrack(this);
}

bool Draggable::Event(ALLEGRO_EVENT& event) {
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
        if (hitCheck(MousePosition(event))) {
            ForceGrab();
            return true;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
        if (gui.IsTracked(this)) {
            Fire(EventType::Moved);
            Dropped();
            gui.UnTrack(this);
            return true;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && gui.IsTracked(this)) {
        Dragged(glm::ivec2(event.mouse.dx, event.mouse.dy));

        return true;
    }
    return false;
}

void Draggable::ForceGrab()
{
    GrabProperties& prop = gui.GrabbedElementProperties();
    prop.location = nullptr;
    prop.position = pos;
    prop.priority = FindMyPriority(GuiElement::Priority::Default);
    gui.Track(this);
    Grabbed();
}
