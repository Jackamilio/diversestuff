#include "JackamikazGUI.h"
#include <algorithm>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_ttf.h>

void jmg::Base::redraw(int origx, int origy)
{
	if (mNeedsRedraw) {
		draw(origx, origy);
		mNeedsRedraw = false;
	}
}

void jmg::Base::draw(int origx, int origy)
{
}

jmg::Base::Base(int relx, int rely, ALLEGRO_COLOR color) : mParent(NULL), mNeedsRedraw(true), mRelx(relx), mRely(rely), mColor(color)
{
}

bool jmg::Base::handleEvent(const ALLEGRO_EVENT & event)
{
	return false;
}

void jmg::Base::addChild(Base * child)
{
	if (child) {
		mChildren.push_back(child);
		child->mParent = this;
	}
}

void jmg::Base::remove()
{
	if (mParent)
	{
		needsRedraw(-1);
		mParent->mChildren.erase(std::find(mParent->mChildren.begin(), mParent->mChildren.end(), this));
	}
}

void jmg::Base::baseDraw()
{
	if (mParent == NULL) {
		cascadeDraw(mRelx,mRely);
	}
}

void jmg::Base::baseHandleEvent(const ALLEGRO_EVENT& event)
{
	if (mParent == NULL) {
		cascadeHandleEvent(event);
	}
}

int jmg::Base::calcOrigX() const
{
	const Base* c = mParent;
	int x = 0;
	while (c) {
		x += c->mRelx;
		c = c->mParent;
	}
	return x;
}

int jmg::Base::calcOrigY() const
{
	const Base* c = mParent;
	int y = 0;
	while (c) {
		y += c->mRely;
		c = c->mParent;
	}
	return y;
}

void jmg::Base::needsRedraw(int depth)
{
	if (depth < 0) {
		depth = 0xFFFFFF;
	}
	Base* farthestParent = this;
	while (farthestParent->mParent && depth-- > 0) {
		farthestParent = farthestParent->mParent;
	}
	farthestParent->mNeedsRedraw = true;
}

void jmg::Base::cascadeDraw(int origx, int origy, bool parentNeedsIt)
{
	bool r = mNeedsRedraw = mNeedsRedraw || parentNeedsIt;
	redraw(origx, origy);
	for (std::vector<Base*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
		(*it)->cascadeDraw(origx+mRelx, origy+mRely, r);
	}
}

bool jmg::Base::cascadeHandleEvent(const ALLEGRO_EVENT& event)
{
	for (std::vector<Base*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
		if ((*it)->cascadeHandleEvent(event)) {
			return true;
		}
	}
	return handleEvent(event);
}

jmg::WallPaper::WallPaper(const ALLEGRO_COLOR & color) : Base(0,0,color)
{
}

void jmg::WallPaper::draw(int, int)
{
	al_clear_to_color(mColor);
}

jmg::MoveableRectangle::MoveableRectangle(int w,int h) : Rectangle(w, h)
{
}

jmg::DrawableRectangle::DrawableRectangle() : mOutline(1)
{
}

jmg::DrawableRectangle::DrawableRectangle(int w, int h) : Rectangle(w,h), mOutline(1)
{
}

void jmg::DrawableRectangle::draw(int origx, int origy)
{
	float x = (float)(origx + mRelx);
	float y = (float)(origy + mRely);
	al_draw_filled_rectangle(x, y, x + (float)mWidth, y + (float)mHeight, mColor);
	if (mOutline) {
		al_draw_rectangle(x, y, x + (float)mWidth, y + (float)mHeight, al_map_rgb(0,0,0), (float)mOutline);
	}
}

jmg::Rectangle::Rectangle() : mWidth(20), mHeight(20)
{
}

jmg::Rectangle::Rectangle(int w, int h) : mWidth(w), mHeight(h)
{
}

jmg::Moveable::Moveable() : mTarget(this), mButton(1)
{
}

jmg::Moveable::Moveable(int w, int h) : Rectangle(w,h), mTarget(this), mButton(1)
{
}

bool jmg::Moveable::handleEvent(const ALLEGRO_EVENT & event)
{
	if (catchMouse(event, mButton)) {
		mDx = event.mouse.x - mTarget->calcOrigX() - mTarget->mRelx;
		mDy = event.mouse.y - mTarget->calcOrigY() - mTarget->mRely;
		mHeld = true;
		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && mHeld) {
		mTarget->mRelx = event.mouse.x - mTarget->calcOrigX() - mDx;
		mTarget->mRely = event.mouse.y - mTarget->calcOrigY() - mDy;

		mTarget->needsRedraw(-1);

		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == mButton) {
		mHeld = false;
	}
	return false;
}

void callbackTest(void* arg) {
	if (arg) {
		jmg::Window* win = (jmg::Window*)arg;
		win->close();
	}
}

jmg::Window::Window(int w, int h, const char* capt) : Rectangle(w,h), mMover(w - 18, 22), mBtnClose(22,22)
{
	mMover.mRelx = -2;
	mMover.mRely = -22;
	mMover.mTarget = this;
	mMover.mColor.r = 0;
	mCaption.mValue = capt;
	mCaption.mRelx = 4;
	mCaption.mRely = 4;
	mMover.addChild(&mCaption);
	mBtnClose.mRelx = mMover.mRelx + mMover.mWidth;
	mBtnClose.mRely = mMover.mRely;
	mBtnClose.mCallback = callbackTest;
	mBtnClose.mCallbackArgs = (void*)this;
	mRelx = 2;
	mRely = 22;

	addChild(&mMover);
	addChild(&mBtnClose);
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
	int dx = px - calcOrigX() - mRelx;
	int dy = py - calcOrigY() - mRely;

	return dx >= 0 && dx < mWidth && dy >= 0 && dy < mHeight;
}

bool jmg::InteractiveRectangle::catchMouse(const ALLEGRO_EVENT & event, int button, ALLEGRO_EVENT_TYPE evType)
{
	// catch mouse
	if (event.type == evType && event.mouse.button == button) {
		int dx = event.mouse.x - calcOrigX() - mRelx;
		int dy = event.mouse.y - calcOrigY() - mRely;

		if (dx >= 0 && dx < mWidth && dy >= 0 && dy < mHeight) {
			return true;
		}
	}
	return false;
}

jmg::Button::Button() : Rectangle(20,20), mHovering(false), mClicking(false), mCallback(NULL), mCallbackArgs(NULL)
{
}

jmg::Button::Button(int w, int h) : Rectangle(w,h), mHovering(false), mClicking(false), mCallback(NULL), mCallbackArgs(NULL)
{
}

void jmg::Button::draw(int origx, int origy)
{
	ALLEGRO_COLOR c = mColor;
	unsigned char a = mHovering ? (mClicking ? 100 : 200) : 255;
	mColor.r = a * mColor.r / 255;
	mColor.g = a * mColor.g / 255;
	mColor.b = a * mColor.b / 255;
	jmg::DrawableRectangle::draw(origx, origy);
	mColor = c;
}

bool jmg::Button::handleEvent(const ALLEGRO_EVENT & event)
{
	if (catchMouse(event)) {
		mClicking = true;
		needsRedraw();
		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
		bool lastHovering = mHovering;
		mHovering = isPointInside(event.mouse.x, event.mouse.y);
		if (mHovering != lastHovering ) {
			needsRedraw();
		}
		return mHovering;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
		bool inside = isPointInside(event.mouse.x, event.mouse.y);
		if (inside && mClicking && mCallback) {
			mCallback(mCallbackArgs);
		}
		mClicking = false;
		mHovering = inside;
		needsRedraw();
		return inside;
	}
	return false;
}

ALLEGRO_FONT * jmg::fetchDefaultFont()
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

int jmg::Label::calcMaxWidth()
{
	return mLimits ? mLimits->mWidth - mRelx : 0xFFFFFF;
}

jmg::Label::Label(const char * val) : Base(0,0,al_map_rgb(0, 0, 0)), mValue(val), mFont(jmg::fetchDefaultFont()), mLimits(NULL)
{
}

void jmg::Label::draw(int origx, int origy)
{
	//al_draw_text(font, color, origx + relx, origy + rely, 0, value.c_str());

	al_draw_multiline_text(mFont,mColor, origx + mRelx, origy + mRely, (float)calcMaxWidth(),
		(float)al_get_font_line_height(mFont), 0, mValue.c_str());
}

jmg::Text::Text(const char * val) : Label(val), mTextPos(0)
{
}

struct TextDrawArgs {
	int charCount;
	int advance;
	int line;
	jmg::Text* text;
};

bool textDrawCallback(int line_num, const char *line, int size, void *extra) {
	TextDrawArgs& args = *((TextDrawArgs*)extra);

	if (args.charCount + size+1 < args.text->mTextPos) {
		args.charCount += size+1;
		return true;
	}
	else {
		args.advance = 0;
		int lastChar = ALLEGRO_NO_KERNING;
		int linePos = args.text->mTextPos - args.charCount;
		for (int i = 0; i <= size && i < linePos; ++i) {
			int newChar = (i >= size) ? ALLEGRO_NO_KERNING : line[i];
			args.advance += al_get_glyph_advance(args.text->mFont, lastChar, newChar);
			lastChar = newChar;
		}

		args.line = line_num * al_get_font_line_height(args.text->mFont);
		return false;
	}
}

void jmg::Text::draw(int origx, int origy)
{
	const int l = 10;

	//int advance = 0;
	//int line = 0;

	TextDrawArgs args;
	args.advance = 0;
	args.charCount = 0;
	args.line = 0;
	args.text = this;

	//
	al_do_multiline_text(mFont, calcMaxWidth(), mValue.c_str(), textDrawCallback, (void*)&args);

	/*/
	const int vl = (int)mValue.length();
	int lastChar = ALLEGRO_NO_KERNING;
	for (int i = 0; i <= vl && i < mTextPos; ++i) {
		int newChar = (i >= vl) ? ALLEGRO_NO_KERNING : mValue[i];
		args.advance += al_get_glyph_advance(mFont, lastChar, newChar);
		lastChar = newChar;
	}//*/

	const float x = (float)(mRelx + calcOrigX() + args.advance);
	const float y = (float)(mRely + calcOrigY() + args.line);

	al_draw_line(x, y, x, y + l * 2, al_map_rgb(255, 0, 0), 2.0f);

	jmg::Label::draw(origx, origy);
}

struct TextClickArgs {
	int x;
	int y;
	int mx;
	int my;
	int textPos;
	jmg::Text* text;
	bool found;
};

bool textClickCallback(int line_num, const char *_line, int size, void *extra) {
	TextClickArgs& args = *((TextClickArgs*)extra);
	std::string line(_line,(size_t)size);

	int bbx, bby, bbw, bbh;
	al_get_text_dimensions(args.text->mFont, line.c_str(), &bbx, &bby, &bbw, &bbh);

	const ALLEGRO_FONT* font = args.text->mFont;

	const int x = args.x;
	const int lh = al_get_font_line_height(font);
	const int y = args.y + line_num * lh;

	const size_t vl = line.length();
	if (bbx + x <= args.mx && args.mx < bbx + x + bbw && y <= args.my && args.my < y + lh) {
		int advance = 0;
		int lastChar = ALLEGRO_NO_KERNING;
		size_t i = 0;
		for (; i <= vl && advance <= args.mx - bbx - x; ++i) {
			int newChar = (i >= vl) ? ALLEGRO_NO_KERNING : line[i];
			advance += al_get_glyph_advance(font, lastChar, newChar);
			lastChar = newChar;
		}

		args.textPos += (int)i - 1;
		args.found = true;
		return false;
	}
	else {
		
		if (y <= args.my && args.my < y + lh) {
			if (args.mx >= bbx + x - args.text->mRelx && args.mx < bbx + x) {
				args.found = true;
				args.textPos += 1;
				return false;
			}
			else if (args.mx <= x + args.text->calcMaxWidth() && args.mx >= bbx + x + bbw) {
				args.found = true;
				args.textPos += vl + 1;
				return false;
			}
		}
		args.textPos += vl + 1;
		return true;
	}
}

bool jmg::Text::handleEvent(const ALLEGRO_EVENT & event)
{
	if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {

		/*int x = calcOrigX() + mRelx;
		int y = calcOrigY() + mRely;
		int mx = event.mouse.x;
		int my = event.mouse.y;*/

		TextClickArgs args;
		args.x = calcOrigX() + mRelx;
		args.y = calcOrigY() + mRely;
		args.mx = event.mouse.x;
		args.my = event.mouse.y;
		args.textPos = 0;
		args.text = this;
		args.found = false;

		al_do_multiline_text(mFont, calcMaxWidth(), mValue.c_str(), textClickCallback, (void*)&args);

		if (args.found) {
			mTextPos = args.textPos;
			needsRedraw(1);
			return true;
		}

		/*int bbx, bby, bbw, bbh;
		al_get_text_dimensions(mFont, mValue.c_str(), &bbx, &bby, &bbw, &bbh);
		if (bbx+x <= mx && mx < bbx+x+bbw && bby+y <= my && my < bby+y+bbh) {
			int advance = 0;
			const size_t vl = mValue.length();
			int lastChar = ALLEGRO_NO_KERNING;
			size_t i = 0;
			for (; i <= vl && advance < mx - bbx - x; ++i) {
				int newChar = (i>=vl) ? ALLEGRO_NO_KERNING : mValue[i];
				advance += al_get_glyph_advance(mFont, lastChar, newChar);
				lastChar = newChar;
			}

			mTextPos = (int)i-2;

			needsRedraw(1);
			return true;
		}*/
	}
	else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
		if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
			if (--mTextPos < 0) {
				mTextPos = 0;
			}
			needsRedraw(1);
			return true;
		}
		else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
			const size_t vl = mValue.length();
			if (++mTextPos > vl && vl > 0) {
				mTextPos = (int)vl;
			}
			needsRedraw(1);
			return true;
		}
	}
	return false;
}
