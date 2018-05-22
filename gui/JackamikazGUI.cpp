#include "JackamikazGUI.h"
#include <map>
#include <algorithm>
#include <allegro5\allegro.h>
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

jmg::Base::Base(int relx, int rely, ALLEGRO_COLOR color)
	: mParent(NULL)
	, mNeedsRedraw(true)
	, mRemoveMe(false)
	, mRelx(relx)
	, mRely(rely)
	, mColor(color)
	, mDeleteMe(false)
{
}

bool jmg::Base::handleEvent(const ALLEGRO_EVENT & event)
{
	return false;
}

bool jmg::Base::has(const Base * child) const
{
	return std::find(mChildren.begin(), mChildren.end(), child) != mChildren.end();
}

void jmg::Base::addChild(Base * child)
{
	if (child && !has(child)) {
		mChildren.push_back(child);
		child->mParent = this;
		child->mRemoveMe = false;
	}
}

void jmg::Base::remove()
{
	if (mParent)
	{
		needsRedraw(-1);
		mRemoveMe = true;
	}
}

void jmg::Base::baseDraw()
{
	if (mParent == NULL) {
		cascadeDraw(mRelx,mRely);
	}
}

bool jmg::Base::baseHandleEvent(const ALLEGRO_EVENT& event)
{
	if (mParent == NULL) {
		return cascadeHandleEvent(event);
	}
	return false;
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

typedef std::list<jmg::Base*> BaseList;

void jmg::Base::cascadeDraw(int origx, int origy, bool parentNeedsIt)
{
	if (!mRemoveMe && !mDeleteMe) {
		bool r = mNeedsRedraw = mNeedsRedraw || parentNeedsIt;
		redraw(origx, origy);
		for (BaseList::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
			(*it)->cascadeDraw(origx + mRelx, origy + mRely, r);
		}
	}
}

bool jmg::Base::cascadeHandleEvent(const ALLEGRO_EVENT& event)
{
	for (BaseList::reverse_iterator it = mChildren.rbegin(); it != mChildren.rend();) {
		bool eventIntercepted = (*it)->cascadeHandleEvent(event);

		bool deleted = false;

		if ((*it)->mDeleteMe) {
			delete (*it);
			deleted = true;
		}

		if (deleted || (*it)->mRemoveMe || (*it)->mParent != this) {
			if (!deleted) {
				(*it)->mRemoveMe = false;
			}
			mChildren.erase(std::next(it).base());
		}
		else {
			++it;
		}

		if (eventIntercepted) {
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

jmg::Window::Window(int w, int h, const char* capt)
	: Rectangle(w,h)
	, mMover(w - 18, 22)
	, mBtnClose(22,22)
	, mBtnImage(Image::CROSS)
{
	mMover.mRelx = -2;
	mMover.mRely = -22;
	mMover.mTarget = this;
	mMover.mColor = al_map_rgb(200,200,200);
	mCaption.setValue(capt);
	mCaption.mRelx = 4;
	mCaption.mRely = 4;
	mMover.addChild(&mCaption);
	mBtnClose.mRelx = mMover.mRelx + mMover.mWidth;
	mBtnClose.mRely = mMover.mRely;
	mBtnClose.mCallback = callbackTest;
	mBtnClose.mCallbackArgs = (void*)this;
	mBtnClose.addChild(&mBtnImage);
	mRelx = 2;
	mRely = 22;

	addChild(&mMover);
	addChild(&mBtnClose);
}

bool jmg::Window::handleEvent(const ALLEGRO_EVENT & event)
{
	return catchMouse(event);
}

void jmg::Window::open()
{
	if (parent()) {
		parent()->addChild(this);
		needsRedraw(-1);
	}
}

void jmg::Window::close()
{
	remove();
	needsRedraw(-1);
}

void jmg::Window::setParent(Base * p, bool startsOpen)
{
	if (p != parent()) {
		p->addChild(this);
		if (!startsOpen) {
			remove();
		}
		needsRedraw(-1);
	}
	else if (p && p->has(this) != startsOpen) {
		startsOpen ? p->addChild(this) : remove();
		needsRedraw(-1);
	}
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

void jmg::Label::setValue(const char * val)
{
	if (!mEditing)
	{
		al_ustr_assign_cstr(mValue, val);
	}
}

void jmg::Label::setValue(const char16_t * val)
{
	if (!mEditing)
	{
		al_ustr_free(mValue);
		mValue = al_ustr_new_from_utf16((const uint16_t*)val);
	}
}

jmg::Label::Label(const char * val)
	: Base(0,0,al_map_rgb(0, 0, 0))
	, mValue(al_ustr_new(val))
	, mEditing(false)
	, mFont(jmg::fetchDefaultFont())
	, mWidth(0xFFFFFF)
{
}

jmg::Label::Label(const char16_t * val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
	, mValue(al_ustr_new_from_utf16((uint16_t*)val))
	, mEditing(false)
	, mFont(jmg::fetchDefaultFont())
	, mWidth(0xFFFFFF)
{
}

jmg::Label::~Label()
{
	al_ustr_free(mValue);
}

void jmg::Label::draw(int origx, int origy)
{
	//al_draw_text(font, color, origx + relx, origy + rely, 0, value.c_str());

	al_draw_multiline_ustr(mFont,mColor, origx + mRelx, origy + mRely, (float)mWidth,
		(float)al_get_font_line_height(mFont), 0, mValue);
}

void jmg::Text::insert(int keycode)
{
	al_ustr_insert_chr(mValue, al_ustr_offset(mValue, mTextPos), keycode);
	mSelectionPos = ++mTextPos;
	needsRedraw(1);
}

struct TextClickArgs {
	int x;
	int y;
	int mx;
	int my;
	int textPos;
	const jmg::Text* text;
	bool found;
};

bool getTextIndexCallback(int line_num, const ALLEGRO_USTR *line, void *extra) {
	TextClickArgs& args = *((TextClickArgs*)extra);

	int bbx, bby, bbw, bbh;
	al_get_ustr_dimensions(args.text->mFont, line, &bbx, &bby, &bbw, &bbh);

	const ALLEGRO_FONT* font = args.text->mFont;

	const int x = args.x;
	const int lh = al_get_font_line_height(font);
	const int y = args.y + line_num * lh;

	const int vl = (int)al_ustr_length(line);
	if (bbx + x <= args.mx && args.mx < bbx + x + bbw && y <= args.my && args.my < y + lh) {
		int advance = 0;
		int lastChar = ALLEGRO_NO_KERNING;
		int i = 0;
		for (; i <= vl && advance <= args.mx - bbx - x; ++i) {
			int newChar = (i >= vl) ? ALLEGRO_NO_KERNING : al_ustr_get(line, al_ustr_offset(line, i));
			advance += al_get_glyph_advance(font, lastChar, newChar);
			lastChar = newChar;
		}

		args.textPos += i - 1;
		args.found = true;
		return false;
	}
	else {

		if (y <= args.my && args.my < y + lh) {
			// clicks on the left of the text
			if (args.mx >= bbx + x - args.text->mRelx && args.mx < bbx + x) {
				args.found = true;
				return false;
			}
			// clicks on the right of the text
			else if (args.mx <= x + args.text->mWidth && args.mx >= bbx + x + bbw) {
				args.found = true;
				args.textPos += vl;
				return false;
			}
		}
		args.textPos += vl + 1;
		return true;
	}
}

int jmg::Text::getTextIndexFromCursorPos(int fromx, int fromy) const
{
	TextClickArgs args;
	args.x = calcOrigX() + mRelx;
	args.y = calcOrigY() + mRely;
	args.mx = fromx;
	args.my = fromy;
	args.textPos = 0;
	args.text = this;
	args.found = false;

	// hack to reach last line if it's a newline
	// because the do_multiline doesn't call the last line when it's the case
	// also if it's empty we still want to be able to clic at least once to edit the text
	const int l = (int)al_ustr_length(mValue);
	bool revertHack = false;
	if (l > 0 && getCharAt(l-1) == '\n' || l == 0) {
		al_ustr_insert_chr(mValue, al_ustr_offset(mValue, l), '.');
		revertHack = true;
	}

	al_do_multiline_ustr(mFont, mWidth, mValue, getTextIndexCallback, (void*)&args);

	if (revertHack) {
		al_ustr_remove_chr(mValue, al_ustr_offset(mValue, l));
	}

	return args.found ? args.textPos : -1;
}

struct TextDrawArgs {
	int charCount;
	int advance; // if it's set negative it's not calculated
	int line; // idem
	int textPos;
	const ALLEGRO_FONT* font;
};

bool getCursorPosCallback(int line_num, const ALLEGRO_USTR *line, void *extra) {
	TextDrawArgs& args = *((TextDrawArgs*)extra);
	const int size = (int)al_ustr_length(line);

	if (args.charCount + size + 1 <= args.textPos) {
		args.charCount += size + 1;
		if (args.line >= 0 && args.charCount == args.textPos) {
			args.line = (line_num+1) * al_get_font_line_height(args.font);
			return false;
		}
		return true;
	}
	else {
		if (args.advance >= 0) {
			args.advance = 0;
			int lastChar = ALLEGRO_NO_KERNING;
			int linePos = args.textPos + 1 - args.charCount;
			for (int i = 0; i <= size && i < linePos; ++i) {
				int newChar = (i >= size) ? ALLEGRO_NO_KERNING : al_ustr_get(line, al_ustr_offset(line, i));
				args.advance += al_get_glyph_advance(args.font, lastChar, newChar);
				lastChar = newChar;
			}
		}

		if (args.line >= 0) {
			args.line = line_num * al_get_font_line_height(args.font);
		}
		return false;
	}
}


void jmg::Text::getCursorPosFromTextIndex(int pos, int * posx, int * posy) const
{
	TextDrawArgs args;
	args.charCount = 0;
	args.advance = posx ? 0 : -1; // avoid unnecessary cpu usage
	args.line = posy ? 0 : -1;
	args.textPos = pos;
	args.font = mFont;

	al_do_multiline_ustr(mFont, mWidth, mValue, getCursorPosCallback, (void*)&args);

	if (posx) *posx = mRelx + calcOrigX() + args.advance;
	if (posy) *posy = mRely + calcOrigY() + args.line;
}

void jmg::Text::resetCursorXRef()
{
	int absxref;
	getCursorPosFromTextIndex(mTextPos, &absxref, NULL);
	cursorXRef = absxref - mRelx - calcOrigX() - 2;
}


jmg::Text::Text(const char * val)
	: Label(val)
	, cursorXRef(-2)
	, mTextPos(0)
	, mSelectionPos(0)
	, mEditCallback(nullptr)
	, mEditCallbackArgs(nullptr)
	, mClicking(false)
	, mIsNumeric(false)
{
}

jmg::Text::Text(const char16_t* val)
	: Label(val)
	, cursorXRef(-2)
	, mTextPos(0)
	, mSelectionPos(0)
	, mEditCallback(nullptr)
	, mEditCallbackArgs(nullptr)
	, mClicking(false)
	, mIsNumeric(false)
{
}

struct DrawSelectionArgs {
	int leftCursor;
	int rightCursor;
	int charCount;
	int absx;
	int absy;
	const ALLEGRO_FONT* font;
};

bool drawSelectionCallback(int line_num, const ALLEGRO_USTR *line, void *extra) {
	DrawSelectionArgs& args = *((DrawSelectionArgs*)extra);
	const int size = (int)al_ustr_length(line) + 1; //+1 for extra space counted at the end of a line

	if (args.charCount + size > args.leftCursor) {
		int advance = 0;
		int leftadvance = 0;
		int lastChar = ALLEGRO_NO_KERNING;
		int linePos = args.leftCursor + 1 - args.charCount;
		int maxPos = args.rightCursor + 1 - args.charCount;
		int max = size < maxPos ? size : maxPos;
		if (linePos < 0) linePos = 0;
		if (max == size) ++max;

		for (int i = 0; i < max; ++i) {
			int newChar = (i >= max) ? ALLEGRO_NO_KERNING : al_ustr_get(line, al_ustr_offset(line, i));
			advance += al_get_glyph_advance(args.font, lastChar, newChar);
			lastChar = newChar;
			if (i < linePos) {
				leftadvance = advance;
			}
		}

		if (advance == 0) {
			advance = al_get_glyph_width(args.font, ' ');
		}

		const int lh = al_get_font_line_height(args.font);
		al_draw_filled_rectangle(
			args.absx + leftadvance, args.absy + line_num * lh,
			args.absx + advance, args.absy + (line_num + 1)*lh,
			al_map_rgb(0, 255, 255));
	}
	args.charCount += size;
	return args.charCount < args.rightCursor;
}

void jmg::Text::draw(int origx, int origy)
{
	if (mEditing) {
		int _x, _y;

		if (mSelectionPos != mTextPos) {
			DrawSelectionArgs args;
			args.leftCursor = mSelectionPos < mTextPos ? mSelectionPos : mTextPos;
			args.rightCursor = mSelectionPos > mTextPos ? mSelectionPos : mTextPos;
			args.charCount = 0;
			args.absx = origx + mRelx;
			args.absy = origy + mRely;
			args.font = mFont;
		
			al_do_multiline_ustr(mFont, mWidth, mValue, drawSelectionCallback, (void*)&args);
		}

		getCursorPosFromTextIndex(mTextPos, &_x, &_y);
		const float x = (float)_x;
		const float y = (float)_y;
		al_draw_line(x, y, x, y + al_get_font_line_height(mFont), al_map_rgb(0, 0, 0), 1.0f);
	}

	jmg::Label::draw(origx, origy);
}

bool jmg::Text::handleCursorPosEvents(const ALLEGRO_EVENT& event) {
	if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1
	 || event.type == ALLEGRO_EVENT_MOUSE_AXES && mClicking) {
		int pos = getTextIndexFromCursorPos(event.mouse.x, event.mouse.y);

		if (pos >= 0) {
			mEditing = true;
			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				mClicking = true;
			}
			if (pos != mTextPos) {
				cursorXRef = event.mouse.x - mRelx - calcOrigX();
				mTextPos = pos;
				needsRedraw(1);
			}
			return true;
		}
		else if (mClicking && event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			return true;
		}
		else {
			confirmEditing();
		}
	}
	else if (mEditing) {
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
			if (mClicking) {
				mClicking = false;
				return true;
			}
		}
		else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
			if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				if (event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) {
					while (getCharAt(--mTextPos) == ' ' && mTextPos > 0) {}
					while (getCharAt(--mTextPos) != ' ' && mTextPos > 0) {}
					if (mTextPos > 0) {
						const int vl = (int)al_ustr_length(mValue);
						if (++mTextPos > vl) {
							mTextPos = (int)vl;
						}
					}
				}
				else if (--mTextPos < 0) {
					mTextPos = 0;
				}

				resetCursorXRef();
				needsRedraw(1);
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				const int vl = (int)al_ustr_length(mValue);

				if (event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) {
					while (getCharAt(++mTextPos) != ' ' && mTextPos < vl) {}
					while (getCharAt(++mTextPos) == ' ' && mTextPos < vl) {}
				}
				else {
					++mTextPos;
				}

				if (mTextPos > vl) {
					mTextPos = (int)vl;
				}
				resetCursorXRef();
				needsRedraw(1);
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
				int x, y;
				getCursorPosFromTextIndex(mTextPos, NULL, &y);
				x = cursorXRef + mRelx + calcOrigX();
				int pos = getTextIndexFromCursorPos(x, y - al_get_font_line_height(mFont));
				mTextPos = pos >= 0 ? pos : 0;
				needsRedraw(1);
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				int x, y;
				getCursorPosFromTextIndex(mTextPos, NULL, &y);
				x = cursorXRef + mRelx + calcOrigX();
				int pos = getTextIndexFromCursorPos(x, y + al_get_font_line_height(mFont));
				if (pos > 0) {
					mTextPos = pos;
					needsRedraw(1);
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_END) {
				int x, y;
				getCursorPosFromTextIndex(mTextPos, NULL, &y);
				const int dif = mWidth;
				x = mRelx + calcOrigX() + dif;
				int pos = getTextIndexFromCursorPos(x, y);
				if (pos > 0) {
					mTextPos = pos;
					needsRedraw(1);
					cursorXRef = dif;
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_HOME) {
				int x, y;
				getCursorPosFromTextIndex(mTextPos, NULL, &y);
				x = calcOrigX();
				int pos = getTextIndexFromCursorPos(x, y);
				mTextPos = pos >= 0 ? pos : 0;
				needsRedraw(1);
				cursorXRef = -2;
				return true;
			}
		}
	}
	return false;
}

bool jmg::Text::collapseSelection()
{
	if (mSelectionPos != mTextPos) {
		int size = mSelectionPos - mTextPos;
		if (size < 0) {
			size = -size;
			mTextPos = mSelectionPos;
		}
		else {
			mSelectionPos = mTextPos;
		}
		al_ustr_remove_range(mValue, al_ustr_offset(mValue, mTextPos), al_ustr_offset(mValue, mTextPos + size));
		return true;
	}
	return false;
}

void jmg::Text::confirmEditing()
{
	mEditing = false;
	if (mEditCallback) {
		mEditCallback(mEditCallbackArgs);
	}
}

bool jmg::Text::handleEvent(const ALLEGRO_EVENT & event)
{
	if (handleCursorPosEvents(event)) {
		ALLEGRO_KEYBOARD_STATE state;
		al_get_keyboard_state(&state);
		const bool shiftmod = al_key_down(&state, ALLEGRO_KEY_LSHIFT) || al_key_down(&state, ALLEGRO_KEY_RSHIFT);
		const bool axes = (event.type == ALLEGRO_EVENT_MOUSE_AXES);
		const bool bdown = (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
		const bool bup = (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP);
		if (!(shiftmod || mClicking && axes || mClicking && shiftmod && bdown || bup)) {
			mSelectionPos = mTextPos;
		}
		return true;
	}
	else if (mEditing) {
		if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
			if (event.keyboard.keycode == ALLEGRO_KEY_A && event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) {
				mSelectionPos = 0;
				mTextPos = (int)al_ustr_length(mValue);
				resetCursorXRef();
				needsRedraw(1);
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
				if (collapseSelection()) {
					resetCursorXRef();
					needsRedraw(1);
				}
				else if (mTextPos > 0) {
					al_ustr_remove_chr(mValue, al_ustr_offset(mValue, --mTextPos));
					mSelectionPos = mTextPos;
					resetCursorXRef();
					needsRedraw(1);
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_DELETE) {
				if (collapseSelection()) {
					resetCursorXRef();
					needsRedraw(1);
				}
				else if (mTextPos < (int)al_ustr_length(mValue)) {
					al_ustr_remove_chr(mValue, al_ustr_offset(mValue, mTextPos));
					needsRedraw(1);
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
				if (mIsNumeric) {
					confirmEditing();
					mSelectionPos = mTextPos;
					needsRedraw(1);
				}
				else {
					collapseSelection();
					insert('\n');
					resetCursorXRef();
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_TAB && !mIsNumeric) {
				collapseSelection();
				// couldn't find a proper tab character that would handle it correctly
				// I could write my own way of drawing a string and treating \t how I want it
				// but it's too much work for low importance
				insert(' ');
				insert(' ');
				insert(' ');
				insert(' ');
				resetCursorXRef();
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				mSelectionPos = mTextPos;
				mEditing = false;
				needsRedraw(1);
				return true;
			}
			else if (event.keyboard.unichar > 0) {
				bool doInsert = !mIsNumeric;
				const int unichar = event.keyboard.unichar;
				if (mIsNumeric) {
					static std::string allowedNumeric("-.012345789");
					if (allowedNumeric.find((char)unichar) != std::string::npos) {
						const int pointPos = al_ustr_find_chr(mValue, 0, '.');
						const int size = (int)al_ustr_size(mValue);
						const int right = mTextPos > mSelectionPos ? mTextPos : mSelectionPos;
						const int minusHere = (al_ustr_find_chr(mValue, 0, '-') < 0 ? 0 : 1);

						if (unichar == '-') {
							doInsert = !mPositiveOnly && !minusHere && (mTextPos == 0 || mSelectionPos == 0);
						}
						else if (unichar == '.') {
							doInsert = pointPos < 0 && size - right <= mMaxDecimals && right - minusHere <= mMaxDigits;
						}
						else {
							const int digits = (pointPos < 0 ? size : pointPos) - minusHere;
							const int decimals = pointPos < 0 ? 0 : size - pointPos - 1;
							doInsert = pointPos >= 0 && right > pointPos ? decimals < mMaxDecimals : digits < mMaxDigits;
						}
					}
				}
				if (doInsert) {
					collapseSelection();
					insert(unichar);
					resetCursorXRef();
					return true;
				}
			}
		}
	}
	return false;
}

jmg::Numeric::Numeric(unsigned char maxDigits, unsigned char maxDecimals, bool positiveOnly)
{
	mIsNumeric = true;
	mMaxDigits = maxDigits;
	mMaxDecimals = maxDecimals;
	mPositiveOnly = positiveOnly;
}


ALLEGRO_BITMAP * jmg::Image::getImage(PreRenderedImage image)
{
	static std::map<PreRenderedImage, ALLEGRO_BITMAP*> images;
	if (!images[image]) {
		ALLEGRO_BITMAP* img = nullptr;
		ALLEGRO_BITMAP* target = al_get_target_bitmap();

		switch (image) {
		case CROSS:
			img = al_create_bitmap(22, 22);
			al_set_target_bitmap(img);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			al_draw_line(4, 4, 18, 18, al_map_rgba(0, 0, 0, 255), 1.5f);
			al_draw_line(18, 4, 4, 18, al_map_rgba(0, 0, 0, 255), 1.5f);
			break;
		default:
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			break;
		}

		al_set_target_bitmap(target);
		images[image] = img;
	}
	return images[image];
}

jmg::Image::Image(const ALLEGRO_COLOR & color) : Base(0,0,color), mImage(nullptr)
{
}

jmg::Image::Image(const char * file, const ALLEGRO_COLOR & color) : Base(0, 0, color), mImage(al_load_bitmap(file))
{
}

jmg::Image::Image(ALLEGRO_BITMAP * bitmap, const ALLEGRO_COLOR & color) : Base(0, 0, color), mImage(bitmap)
{
}

jmg::Image::Image(PreRenderedImage image, const ALLEGRO_COLOR & color) : Base(0, 0, color), mImage(getImage(image))
{
}

void jmg::Image::draw(int origx, int origy)
{
	if (mImage) {
		al_draw_tinted_bitmap(mImage, mColor, origx + mRelx, origy + mRely, 0);
	}
}
