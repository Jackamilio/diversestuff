#include "Window.h"
#include "GuiMaster.h"
#include "DefaultColors.h"

Window::Window() :
	GuiElement(true),
	headBandHeight(20),
	horiResize(nullptr),
	vertResize(nullptr),
	minWidth(50),
	minHeight(20)
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

#undef min
#undef max

Engine::InputStatus Window::Event(ALLEGRO_EVENT& event)
{
	const bool leftClicked = event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1;
	const bool leftReleased = event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1;
	const bool axes = event.type == ALLEGRO_EVENT_MOUSE_AXES;
	if (leftClicked || leftReleased || axes) {
		const int dist = 2;
		const int _t = top - headBandHeight;
		bool insideHori = valueInside(event.mouse.x, l - dist, r + dist);
		bool insideVert = valueInside(event.mouse.y, _t - dist, b + dist);
		bool nearL = insideVert && glm::abs(event.mouse.x - l) <= dist;
		bool nearR = insideVert && glm::abs(event.mouse.x - r) <= dist;
		bool nearT = insideHori && glm::abs(event.mouse.y - _t) <= dist;
		bool nearB = insideHori && glm::abs(event.mouse.y - b) <= dist;
		char nearLRTB = 0b0;
		if (nearL) nearLRTB |= 0b0001;
		if (nearR) nearLRTB |= 0b0010;
		if (nearT) nearLRTB |= 0b0100;
		if (nearB) nearLRTB |= 0b1000;
		switch (nearLRTB) {
		case 0b0000:
			gui.CancelCursor(this);
			break;
		case 0b0001:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_W);
			break;
		case 0b0010:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_E);
			break;
		case 0b0100:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_N);
			break;
		case 0b1000:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_S);
			break;
		case 0b0101:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NW);
			break;
		case 0b0110:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NE);
			break;
		case 0b1001:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_SW);
			break;
		case 0b1010:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_SE);
			break;
		default:
			gui.RequestCursor(this, ALLEGRO_SYSTEM_MOUSE_CURSOR_QUESTION);
			break;
		}
		if (leftClicked) {
			horiResize = nearL ? &left : nearR ? &right  : nullptr;
			vertResize = nearT ? &top  : nearB ? &bottom : nullptr;
			if (horiResize) trackedHoriResize = *horiResize;
			if (vertResize) trackedVertResize = *vertResize;
		}
		else if (leftReleased) {
			horiResize = nullptr;
			vertResize = nullptr;
		}
		else if (axes) {
			trackedHoriResize += event.mouse.dx;
			trackedVertResize += event.mouse.dy;
			Rect ref = *this;
			if (horiResize) {
				*horiResize = trackedHoriResize;
				if (ref.l != l) l = glm::min(l, ref.r - minWidth);
				else if (ref.r != r) r = glm::max(r, ref.l + minWidth);
			}
			if (vertResize) {
				*vertResize = trackedVertResize;
				if (ref.t != t) top = glm::min(t, ref.b - minHeight);
				else if (ref.b != b) b = glm::max(b, ref.t + minHeight);
			}
			if (horiResize || vertResize) return Engine::InputStatus::grabbed;
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
