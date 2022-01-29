#include "Window.h"
#include "DefaultColors.h"

Window::Window() :
	headBandHeight(20)
{
	cropping = this;
}

Window::~Window() {
}

Rect Window::HeadBandRect() const
{
	Rect ret(*this);
	ret.tl.y -= headBandHeight;
	ret.br.y = tl.y;
	return ret;
}

void Window::Draw() {
	Rect head = HeadBandRect();
	head.draw_filled(darkgrey);
	head.draw(white, 1);

	draw_filled(black);

	CropperDisplacer::Draw();
}

void Window::PostDraw() {
	CropperDisplacer::PostDraw();
	draw(white, 1);
}

Engine::InputStatus Window::Event(ALLEGRO_EVENT& event)
{
	if (CropperDisplacer::Event(event) != Engine::InputStatus::grabbed) {
		if (Draggable::Event(event) == Engine::InputStatus::ignored) {
			return Engine::InputStatus::ignored;
		}
	}
	return Engine::InputStatus::notforchildren;
}

void Window::Dragged(const glm::ivec2& delta)
{
	*this += delta;
}

bool Window::hitCheck(const glm::ivec2& pos) const
{
	return HeadBandRect().isInside(pos);
}

void Window::SetPos(const glm::ivec2& tsl)
{
	*this += tsl - topleft;
}

glm::ivec2 Window::GetPos() const
{
	return topleft;
}

glm::ivec2 Window::GetDisplaceOffset() const
{
	return topleft;
}
