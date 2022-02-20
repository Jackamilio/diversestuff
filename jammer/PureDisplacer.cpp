#include "PureDisplacer.h"

PureDisplacer::PureDisplacer()// : GuiElement(true)
{
}

Engine::InputStatus PureDisplacer::Event(ALLEGRO_EVENT& event)
{
	Engine::InputStatus cropperret = CropperDisplacer::Event(event);
	if (cropperret != Engine::InputStatus::grabbed) {
		return Draggable::Event(event) == Engine::InputStatus::grabbed ? Engine::InputStatus::grabbed : cropperret;
	}
	else {
		return Engine::InputStatus::ignored;
	}
}

glm::ivec2 PureDisplacer::GetDisplaceOffset() const
{
	return pos;
}

bool PureDisplacer::hitCheck(const glm::ivec2& pos) const
{
	return true;
}
