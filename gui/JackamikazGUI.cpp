#include "JackamikazGUI.h"
#include <algorithm>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_ttf.h>

void jmg::Base::redraw(int origx, int origy)
{
	if (needsRedraw) {
		draw(origx, origy);
		needsRedraw = false;
	}
}

void jmg::Base::draw(int origx, int origy)
{
}

jmg::Base::Base() : parent(NULL), needsRedraw(true), relx(0), rely(0)
{
}

bool jmg::Base::handleEvent(const ALLEGRO_EVENT & event)
{
	return false;
}

void jmg::Base::addChild(Base * child)
{
	if (child) {
		children.push_back(child);
		child->parent = this;
	}
}

void jmg::Base::remove()
{
	if (parent)
	{
		moved();
		parent->children.erase(std::find(parent->children.begin(), parent->children.end(), this));
	}
}

void jmg::Base::baseDraw()
{
	if (parent == NULL) {
		cascadeDraw(relx,rely);
	}
}

void jmg::Base::baseHandleEvent(const ALLEGRO_EVENT& event)
{
	if (parent == NULL) {
		cascadeHandleEvent(event);
	}
}

int jmg::Base::calcOrigX() const
{
	const Base* c = parent;
	int x = 0;
	while (c) {
		x += c->relx;
		c = c->parent;
	}
	return x;
}

int jmg::Base::calcOrigY() const
{
	const Base* c = parent;
	int y = 0;
	while (c) {
		y += c->rely;
		c = c->parent;
	}
	return y;
}

void jmg::Base::moved()
{
	Base* farthestParent = this;
	while (farthestParent->parent) {
		farthestParent = farthestParent->parent;
	}
	farthestParent->needsRedraw = true;
}

void jmg::Base::cascadeDraw(int origx, int origy, bool parentNeedsIt)
{
	bool r = needsRedraw = needsRedraw || parentNeedsIt;
	redraw(origx, origy);
	for (std::vector<Base*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->cascadeDraw(origx+relx, origy+rely, r);
	}
}

bool jmg::Base::cascadeHandleEvent(const ALLEGRO_EVENT& event)
{
	for (std::vector<Base*>::iterator it = children.begin(); it != children.end(); ++it) {
		if ((*it)->cascadeHandleEvent(event)) {
			return true;
		}
	}
	return handleEvent(event);
}

jmg::WallPaper::WallPaper(const ALLEGRO_COLOR & color) : color(color)
{
}

void jmg::WallPaper::draw(int, int)
{
	al_clear_to_color(color);
}

jmg::MoveableRectangle::MoveableRectangle(int w,int h) : Rectangle(w, h)
{
}

jmg::DrawableRectangle::DrawableRectangle() : color(al_map_rgb(255, 255, 255)), outline(1)
{
}

jmg::DrawableRectangle::DrawableRectangle(int w, int h) : Rectangle(w,h), color(al_map_rgb(255,255,255)), outline(1)
{
}

void jmg::DrawableRectangle::draw(int origx, int origy)
{
	float x = (float)(origx + relx);
	float y = (float)(origy + rely);
	al_draw_filled_rectangle(x, y, x + (float)width, y + (float)height, color);
	if (outline) {
		al_draw_rectangle(x, y, x + (float)width, y + (float)height, al_map_rgb(0,0,0), (float)outline);
	}
}

jmg::Rectangle::Rectangle() : width(20), height(20)
{
}

jmg::Rectangle::Rectangle(int w, int h) : width(w), height(h)
{
}

jmg::Moveable::Moveable() : target(this), button(1)
{
}

jmg::Moveable::Moveable(int w, int h) : Rectangle(w,h), target(this), button(1)
{
}

bool jmg::Moveable::handleEvent(const ALLEGRO_EVENT & event)
{
	if (catchMouse(event, button)) {
		dx = event.mouse.x - target->calcOrigX() - target->relx;
		dy = event.mouse.y - target->calcOrigY() - target->rely;
		held = true;
		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && held) {
		target->relx = event.mouse.x - target->calcOrigX() - dx;
		target->rely = event.mouse.y - target->calcOrigY() - dy;

		target->moved();

		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == button) {
		held = false;
	}
	return false;
}

void callbackTest(void* arg) {
	if (arg) {
		jmg::Window* win = (jmg::Window*)arg;
		win->close();
	}
}

jmg::Window::Window(int w, int h, const char* capt) : Rectangle(w,h), mover(w - 18, 22), btnClose(22,22)
{
	mover.relx = -2;
	mover.rely = -22;
	mover.target = this;
	mover.color.r = 0;
	caption.value = capt;
	caption.relx = 4;
	caption.rely = 4;
	mover.addChild(&caption);
	btnClose.relx = mover.relx + mover.width;
	btnClose.rely = mover.rely;
	btnClose.callback = callbackTest;
	btnClose.args = (void*)this;
	relx = 2;
	rely = 22;

	addChild(&mover);
	addChild(&btnClose);
}

bool jmg::Window::handleEvent(const ALLEGRO_EVENT & event)
{
	return catchMouse(event);
}

void jmg::Window::close()
{
	remove();
}

jmg::InteractiveRectangle::InteractiveRectangle()
{
}

jmg::InteractiveRectangle::InteractiveRectangle(int w, int h) : Rectangle(w,h)
{
}

bool jmg::InteractiveRectangle::isPointInside(int px, int py)
{
	int dx = px - calcOrigX() - relx;
	int dy = py - calcOrigY() - rely;

	return dx >= 0 && dx < width && dy >= 0 && dy < height;
}

bool jmg::InteractiveRectangle::catchMouse(const ALLEGRO_EVENT & event, int button, ALLEGRO_EVENT_TYPE evType)
{
	// catch mouse
	if (event.type == evType && event.mouse.button == button) {
		int dx = event.mouse.x - calcOrigX() - relx;
		int dy = event.mouse.y - calcOrigY() - rely;

		if (dx >= 0 && dx < width && dy >= 0 && dy < height) {
			return true;
		}
	}
	return false;
}

jmg::Button::Button() : Rectangle(20,20), hovering(false), clicking(false), callback(NULL), args(NULL)
{
}

jmg::Button::Button(int w, int h) : Rectangle(w,h), hovering(false), clicking(false), callback(NULL), args(NULL)
{
}

void jmg::Button::draw(int origx, int origy)
{
	ALLEGRO_COLOR c = color;
	unsigned char a = hovering ? (clicking ? 100 : 200) : 255;
	color.r = a * color.r / 255;
	color.g = a * color.g / 255;
	color.b = a * color.b / 255;
	jmg::DrawableRectangle::draw(origx, origy);
	color = c;
}

bool jmg::Button::handleEvent(const ALLEGRO_EVENT & event)
{
	if (catchMouse(event)) {
		clicking = true;
		needsRedraw = true;
		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
		bool lastHovering = hovering;
		hovering = isPointInside(event.mouse.x, event.mouse.y);
		needsRedraw = hovering != lastHovering;
		return hovering;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
		bool inside = isPointInside(event.mouse.x, event.mouse.y);
		if (inside && clicking && callback) {
			callback(args);
		}
		clicking = false;
		hovering = inside;
		needsRedraw = true;
		return inside;
	}
	return false;
}

ALLEGRO_FONT * jmg::FetchDefaultFont()
{
	static ALLEGRO_FONT * defaultFont = NULL;
	if (!defaultFont) {
		defaultFont = al_load_ttf_font("arial.ttf", 14, 0);
		if (!defaultFont) {
			defaultFont = al_create_builtin_font();
		}
	}
	return defaultFont;
}

jmg::Label::Label(const char * val) : value(val), color(al_map_rgb(0,0,0)), font(jmg::FetchDefaultFont())
{
}

void jmg::Label::draw(int origx, int origy)
{
	//al_draw_text(font, color, origx + relx, origy + rely, 0, value.c_str());

	al_draw_multiline_text(font,color, origx + relx, origy + rely, 9999.9f,
		(float)al_get_font_line_height(font), 0, value.c_str());
}

jmg::Text::Text(const char * val) : Label(val), textPos(0)
{
}

void jmg::Text::draw(int origx, int origy)
{
	const int l = 10;
	al_draw_line(tx - l, ty, tx + l, ty, color, 1.0f);
	al_draw_line(tx, ty - l, tx, ty + l, color, 1.0f);

	int advance = 0;
	int line = 0;

	const size_t vl = value.length();
	int lastChar = ALLEGRO_NO_KERNING;
	for (size_t i = 0; i < vl && i < textPos; ++i) {
		int newChar = value[i];
		if (lastChar == '\n') {
			advance = 0;
			line += al_get_font_line_height(font);
		}
		else {
			advance += al_get_glyph_advance(font, lastChar, newChar);
		}
		lastChar = newChar;
	}

	advance += al_get_glyph_advance(font, lastChar, ALLEGRO_NO_KERNING);

	const float x = (float)(relx + calcOrigX() + advance);
	const float y = (float)(rely + calcOrigY() + line);

	al_draw_line(x, y, x, y + l * 2, al_map_rgb(255, 0, 0), 2.0f);

	jmg::Label::draw(origx, origy);
}

bool jmg::Text::handleEvent(const ALLEGRO_EVENT & event)
{
	if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
		int bbx, bby, bbw, bbh;
		al_get_text_dimensions(jmg::FetchDefaultFont(), value.c_str(), &bbx, &bby, &bbw, &bbh);

		int x = calcOrigX() + relx;
		int y = calcOrigY() + rely;
		int mx = event.mouse.x;
		int my = event.mouse.y;

		if (bbx+x <= mx && mx < bbx+x+bbw && bby+y <= my && my < bby+y+bbh) {
			tx = mx;
			ty = my;
			moved();
			return true;
		}
	}
	else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
		if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
			if (--textPos < 0) {
				textPos = 0;
			}
			moved();
			return true;
		}
		else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
			const size_t vl = value.length();
			if (++textPos > vl && vl > 0) {
				textPos = (int)vl;
			}
			moved();
			return true;
		}
	}
	return false;
}
