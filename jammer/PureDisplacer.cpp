#include "PureDisplacer.h"

PureDisplacer::PureDisplacer() : GuiElement(true), offset{}
{
}

Engine::InputStatus PureDisplacer::Event(ALLEGRO_EVENT& event)
{
	if (CropperDisplacer::Event(event) != Engine::InputStatus::grabbed) {
		return Draggable::Event(event) == Engine::InputStatus::grabbed ? Engine::InputStatus::grabbed : Engine::InputStatus::notforchildren;
	}
	else {
		return Engine::InputStatus::notforchildren;
	}
}

glm::ivec2 PureDisplacer::GetDisplaceOffset() const
{
	return offset;
}

void PureDisplacer::SetPos(const glm::ivec2& tsl)
{
	offset = tsl;
}

glm::ivec2 PureDisplacer::GetPos() const
{
	return offset;
}

void PureDisplacer::Dragged(const glm::ivec2& delta)
{
	offset += delta;
}

bool PureDisplacer::hitCheck(const glm::ivec2& pos) const
{
	return true;
}
