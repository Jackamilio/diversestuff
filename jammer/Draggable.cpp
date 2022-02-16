#include "Draggable.h"
#include "CropperDisplacer.h"
#include "GuiMaster.h"

void Draggable::CancelGrab()
{
    gui.UnTrack(this);
}

Engine::InputStatus Draggable::Event(ALLEGRO_EVENT& event) {
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
        if (hitCheck(glm::ivec2(event.mouse.x, event.mouse.y))) {
            ForceGrab();
            return Engine::InputStatus::grabbed;
        }
    }
    else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
        if (gui.IsTracked(this)) {
            Fire(EventType::Moved);
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

void Draggable::ForceGrab()
{
    GrabProperties& prop = gui.GrabbedElementProperties();
    prop.location = nullptr;
    prop.position = GetPos();
    prop.priority = FindMyPriority(GuiElement::Priority::Default);
    gui.Track(this);
    Grabbed();
}
