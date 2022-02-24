#include "Window.h"
#include "GuiMaster.h"
#include "DefaultColors.h"

static const int headBandHeight = 20;

Window::Window() :
	Cropper(*((Rect*)this)), // if I don't EXPLICITLY cast "this" to Rect*, the Cropper constructor WON'T BE CALLED AT ALL WITHOUT ANY WARNING, WHAT THE ACTUAL F****, TELL ME SOMETHING COMPILER!!!!!
	horiResize(nullptr),
	vertResize(nullptr),
	trackedHoriResize(0),
	trackedVertResize(0),
	minWidth(50),
	minHeight(20)
{
}

Window::~Window() {
}

Rect Window::HeadBandRect() const
{
	Rect ret(*this);
	ret.top -= headBandHeight;
	ret.bottom = top;
	return ret;
}

void Window::Draw() {
	Rect head = HeadBandRect();
	head.draw_filled(darkgrey);
	head.draw(white, 1);

	Rect::draw_filled(black);

	Cropper::Draw();
}

void Window::PostDraw() {
	Cropper::PostDraw();
	Rect::draw(white, 1);
}

#undef min
#undef max

bool Window::Event(ALLEGRO_EVENT& event)
{
	const bool leftClicked = event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1;
	const bool leftReleased = event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1;
	const bool axes = event.type == ALLEGRO_EVENT_MOUSE_AXES;
	bool mouseInside = false;
	if (leftClicked || leftReleased || axes) {
		const int dist = 2;
		const int _t = top - headBandHeight;
		bool insideHori = valueInside(event.mouse.x, l - dist, r + dist);
		bool insideVert = valueInside(event.mouse.y, _t - dist, b + dist);
		mouseInside = insideHori && insideVert;
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
			gui.CancelCursor(ALLEGRO_EVENT_DISPLAY_RESIZE); // I put this as an ID because : 1) Too lazy to make an enum, 2) this makes enough sense
			break;
		case 0b0001:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_W);
			break;
		case 0b0010:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_E);
			break;
		case 0b0100:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_N);
			break;
		case 0b1000:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_S);
			break;
		case 0b0101:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NW);
			break;
		case 0b0110:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NE);
			break;
		case 0b1001:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_SW);
			break;
		case 0b1010:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_SE);
			break;
		default:
			gui.RequestCursor(ALLEGRO_EVENT_DISPLAY_RESIZE, ALLEGRO_SYSTEM_MOUSE_CURSOR_QUESTION);
			break;
		}
		if (leftClicked) {
			horiResize = nearL ? &left : nearR ? &right  : nullptr;
			vertResize = nearT ? &top  : nearB ? &bottom : nullptr;
			if (horiResize) trackedHoriResize = *horiResize;
			if (vertResize) trackedVertResize = *vertResize;
			if (mouseInside) PutOnTop();
		}
		else if (leftReleased) {
			if (horiResize || vertResize) {
				horiResize = nullptr;
				vertResize = nullptr;
				Fire(EventType::Resized);
			}
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
				if (ref.t != t) t = glm::min(t, ref.b - minHeight);
				else if (ref.b != b) b = glm::max(b, ref.t + minHeight);
			}
			if (horiResize || vertResize) return true;
		}
	}

	return Draggable::Event(event) || Cropper::Event(event) || mouseInside;
}

void Window::Grabbed()
{
	PutOnTop();
}

bool Window::hitCheck(const glm::ivec2& pos) const
{
	return HeadBandRect().isInside(pos);
}

