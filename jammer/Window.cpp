#include "Window.h"
#include "GuiMaster.h"
#include "DefaultColors.h"

Window::Window() :
	GuiElement(true),
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

bool vecsAreClose(const glm::ivec2 v1, const glm::ivec2 v2) {
	glm::ivec2 atob(v1 - v2);
	return atob.x * atob.x + atob.y * atob.y <= 25;
}

Engine::InputStatus Window::Event(ALLEGRO_EVENT& event)
{
	if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
		glm::ivec2 mpos(event.mouse.x, event.mouse.y);
		if (vecsAreClose(mpos, topleft)) {
			gui.SetCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NW);
		}
		else {
			//gui.SetCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
		}
	}
	if (CropperDisplacer::Event(event) == Engine::InputStatus::grabbed) {
		return Engine::InputStatus::grabbed;
	}
	else if (Draggable::Event(event) == Engine::InputStatus::grabbed) {
			return Engine::InputStatus::grabbed;
	}
	return Engine::InputStatus::notforchildren;
}

void Window::Grabbed()
{
	PutOnTop();
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
