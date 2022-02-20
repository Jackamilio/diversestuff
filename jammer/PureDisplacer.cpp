#include "PureDisplacer.h"

/*PureDisplacer::PureDisplacer()// : GuiElement(true)
{
}

bool PureDisplacer::Event(ALLEGRO_EVENT& event)
{
	bool cropperret = CropperDisplacer::Event(event);
	if (cropperret != bool::grabbed) {
		return Draggable::Event(event) == bool::grabbed ? bool::grabbed : cropperret;
	}
	else {
		return false;
	}
}*/

bool PureDisplacer::hitCheck(const glm::ivec2& pos) const
{
	return true;
}
