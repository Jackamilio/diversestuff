#include "JackamikazGUI.h"
#include <map>
#include <stack>
#include <algorithm>
#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_ttf.h>
#include <limits>

void jmg::Base::redraw(int origx, int origy)
{
	if (mNeedsRedraw) {
		draw(origx, origy);
		mNeedsRedraw = false;
	}
}

jmg::Context& jmg::Base::getContext()
{
	static Context context;
	return context;
}

void jmg::Base::draw(int origx, int origy)
{
}

jmg::Base::Base(int relx, int rely, ALLEGRO_COLOR color)
	: mParent(nullptr)
	, mNeedsRedraw(true)
	, mRemoveMe(false)
	, mRelx(relx)
	, mRely(rely)
	, mColor(color)
	, mDeleteMe(false)
{
}

jmg::Base::~Base()
{
	for (auto child : mChildren) {
		child->mParent = nullptr;
		// onRemoveChild(*it); // not sure if it shoud be called
	}
}

bool jmg::Base::handleEvent(const ALLEGRO_EVENT & event)
{
	return false;
}

void jmg::Base::triggerEvent(EventCallback::Type type, const EventCallback::Details& details)
{
	for (auto ev : mEventSubs[type]) {
		ev.mFunction(details, ev.mArgs);
	}
}

bool jmg::Base::has(const Base * child) const
{
	return std::find(mChildren.begin(), mChildren.end(), child) != mChildren.end();
}

void jmg::Base::addChild(Base * child)
{
	if (child && !has(child)) {
		mChildren.push_back(child);
		if (child->mParent) {
			child->remove(); // orphan from old parent
		}
		child->mParent = this;
		child->mRemoveMe = false;
		//onAddChild(child);
		triggerEvent(EventCallback::childAdded, { this, child } );
		child->triggerEvent(EventCallback::added, { child, this });
	}
}

void jmg::Base::addChild(Base * child, int relx, int rely)
{
	child->mRelx = relx;
	child->mRely = rely;
	addChild(child);
}

void jmg::Base::addChild(Base * child, Edge pos, int xory)
{
	int edge = getEdge(pos);

	int add = 0;
	switch (pos) {
	case TOP:
		add = -child->getHeight();
	case BOTTOM:
		child->mRelx = xory;
		child->mRely = edge + add;
		break;
	case LEFT:
		add = 0; // todo -child->getWidth();
	case RIGHT:
		child->mRelx = edge + add;
		child->mRely = xory;
		break;
	}
	
	addChild(child);
}

int jmg::Base::getEdge(Edge edge) const
{
	int Base::*compVal;
	int Base::*otherVal;
	bool lessThan = false;

	switch (edge) {
	case TOP:
		lessThan = true;
	case BOTTOM:
		compVal = &Base::mRely;
		otherVal = &Base::mRelx;
		break;
	case LEFT:
		lessThan = true;
	case RIGHT:
		compVal = &Base::mRelx;
		otherVal = &Base::mRely;
		break;
	}

	int comp = 0;
	Base* bComp = nullptr;
	for (auto obj : mChildren) {
		if (!obj->mRemoveMe && (lessThan ? obj->*compVal <= comp : obj->*compVal >= comp)) {
			comp = obj->*compVal;
			bComp = obj;
		}
	}

	int add = 0;
	if (bComp) {
		if (BOTTOM) { add = bComp->getHeight(); }
		else if (RIGHT) {
			add = 0;// todo add = bComp->getWidth();
		}
	}

	return comp + add;
}

void jmg::Base::setAsAutoAddRef(int startx, int starty, int additionalMargin)
{
	getContext().mAutoAdd = { this, startx,starty,additionalMargin };
}

void jmg::Base::autoAdd(Base* parent)
{
	Context::AutoAdd& aa = parent->getContext().mAutoAdd;
	if (!parent) {
		parent = aa.mReference;
	}
	if (parent) {
		int x = aa.mRelx;
		int y = aa.mRely;

		Base* curRef = aa.mReference;
		while (curRef && curRef != parent) {
			x += curRef->mRelx;
			y += curRef->mRely;
			curRef = curRef->mParent;
		}

		parent->addChild(this, x, y);
		aa.mRely += getHeight() + aa.mAddititonalMargin;
	}
}

void jmg::Base::autoAddShift(int shiftx, int shifty)
{
	Context::AutoAdd& aa = getContext().mAutoAdd;
	aa.mRelx += shiftx;
	aa.mRely += shifty;
}

typedef std::list<jmg::Base*> BaseList;

struct RemoveData {
	jmg::Base* remover;
	BaseList::iterator it;
};

std::stack<RemoveData> toRemove;

void jmg::Base::remove(bool del)
{
	if (mParent)
	{
		requestRedraw(-1);
		mRemoveMe = true;
		mDeleteMe = del;
		toRemove.push({ mParent, std::find(mParent->mChildren.begin(), mParent->mChildren.end(), this) });
		mParent = nullptr;
	}
}

void jmg::Base::baseDraw()
{
	if (mParent == nullptr) {
		purgeRemoveList();
		cascadeDraw(0, 0);
	}
}

void jmg::Base::purgeRemoveList() {
	static int callId = 0;
	++callId;
	if (callId == 1) {
		while (!toRemove.empty()) {
			RemoveData& rem = toRemove.top();
			Base* obj = (Base*)*rem.it;

			rem.remover->triggerEvent(EventCallback::childRemoved, { rem.remover , obj });
			obj->triggerEvent(EventCallback::removed, { obj, rem.remover });

			obj->mRemoveMe = false;
			rem.remover->mChildren.erase(rem.it);

			if (obj->mDeleteMe) {
				delete obj;
			}
			toRemove.pop();
		}
	}
	--callId;
}

bool jmg::Base::baseHandleEvent(const ALLEGRO_EVENT& event)
{
	if (mParent == nullptr) {
		bool ret = cascadeHandleEvent(event);
		purgeRemoveList();
		return ret;
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

void jmg::Base::requestRedraw(int depth)
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

void jmg::Base::subscribeToEvent(EventCallback::Type type, EventCallback callback)
{
	mEventSubs[type].push_back(callback);
}

void jmg::Base::unsubscribeToEvent(EventCallback::Type type, EventCallback callback)
{
	std::list<EventCallback>& l = mEventSubs[type];
	l.erase(std::find(l.begin(), l.end(), callback));
}

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
	for (BaseList::reverse_iterator it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
		if ((*it)->cascadeHandleEvent(event)) {
			return true;
		}
	}

	return handleEvent(event);
}

jmg::MoveableDrawableRectangle::MoveableDrawableRectangle(int w,int h)
	: MoveableRectangle(w, h)
	, DrawableRectangle(w, h)
	, InteractiveRectangle(w, h)
{
}

jmg::DrawableRectangle::DrawableRectangle(int w, int h) : InteractiveRectangle(w,h), mOutline(1)
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

jmg::Rectangle::Rectangle(int w, int h) : mWidth(w), mHeight(h)
{
}

bool jmg::Moveable::catchCondition(const ALLEGRO_EVENT & event) const
{
	return event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
		&& (event.mouse.button == mButton || mButton < 0);
}

jmg::Moveable::Moveable()
	: mTarget(this)
	, mButton(1)
{
}

bool jmg::Moveable::handleEvent(const ALLEGRO_EVENT & event)
{
	if (catchCondition(event)) {
		mDx = event.mouse.x - mTarget->calcOrigX() - mTarget->mRelx;
		mDy = event.mouse.y - mTarget->calcOrigY() - mTarget->mRely;
		mHeld = true;
		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && mHeld) {
		mTarget->mRelx = event.mouse.x - mTarget->calcOrigX() - mDx;
		mTarget->mRely = event.mouse.y - mTarget->calcOrigY() - mDy;

		mTarget->requestRedraw(-1);

		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == mButton) {
		mHeld = false;
	}
	return false;
}

int jmg::Moveable::getHeight() const
{
	return 0;
}

bool jmg::MoveableRectangle::catchCondition(const ALLEGRO_EVENT & event) const
{
	return catchMouse(event, mButton);
}

jmg::MoveableRectangle::MoveableRectangle(int w, int h)
	: InteractiveRectangle(w, h)
{
}

int jmg::MoveableRectangle::getHeight() const
{
	return InteractiveRectangle::getHeight();
}


void callbackTest(const	jmg::EventCallback::Details& details, void* arg) {
	if (arg) {
		jmg::Window* win = (jmg::Window*)arg;
		win->close();
	}
}

jmg::Window::Window(int w, int h, const char* capt)
	: InteractiveRectangle(w,h)
	, DrawableRectangle(w, h)
	, mContext(nullptr)
	, mMover(mWidth - 18, 22)
	, mCaption(capt)
	, mBtnClose(22,22)
	, mBtnImage(Image::CROSS)
{
	mMover.mRelx = -2;
	mMover.mRely = -22;
	mMover.mTarget = this;
	mMover.mColor = al_map_rgb(200,200,200);
	mCaption.mRelx = 4;
	mCaption.mRely = 4;
	mMover.addChild(&mCaption);
	mBtnClose.mRelx = mMover.mRelx + mMover.mWidth;
	mBtnClose.mRely = mMover.mRely;
	//mBtnClose.mCallback = callbackTest;
	//mBtnClose.mCallbackArgs = (void*)this;
	mBtnClose.subscribeToEvent(EventCallback::clicked, { callbackTest, (void*)this });
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
	if (mContext && !parent()) {
		mContext->addChild(this);
		requestRedraw(-1);
	}
}

void jmg::Window::close()
{
	remove();
	requestRedraw(-1);
}

void jmg::Window::setContext(Base * context, bool startsOpen)
{
	mContext = context;
	if (startsOpen) {
		open();
	}
}

jmg::InteractiveRectangle::InteractiveRectangle(int w, int h) : Rectangle(w,h)
{
}

bool jmg::InteractiveRectangle::isPointInside(int px, int py) const
{
	int dx = px - calcOrigX() - mRelx;
	int dy = py - calcOrigY() - mRely;

	return dx >= 0 && dx < mWidth && dy >= 0 && dy < mHeight;
}

bool jmg::InteractiveRectangle::catchMouse(const ALLEGRO_EVENT & event, int button, ALLEGRO_EVENT_TYPE evType) const
{
	// catch mouse
	if (event.type == evType && (event.mouse.button == button || button < 0)) {
		int dx = event.mouse.x - calcOrigX() - mRelx;
		int dy = event.mouse.y - calcOrigY() - mRely;

		if (dx >= 0 && dx < mWidth && dy >= 0 && dy < mHeight) {
			return true;
		}
	}
	return false;
}

void jmg::InteractiveRectangle::addAndAdaptLabel(Label * label, int leftMargin, int topMargin, int rightMargin)
{
	if (topMargin < 0) {
		topMargin = leftMargin;
	}
	if (rightMargin < 0) {
		rightMargin = leftMargin;
	}
	label->mRelx = leftMargin;
	label->mRely = topMargin;
	label->mWidth = mWidth - leftMargin - rightMargin;
	addChild(label);
}

int jmg::InteractiveRectangle::getHeight() const
{
	return mHeight;
}

jmg::Button::Button()
	: InteractiveRectangle(20,20)
	, DrawableRectangle(20,20)
	, mHovering(false)
	, mClicking(false)
//	, mCallback(NULL)
//	, mCallbackArgs(NULL)
{
}

jmg::Button::Button(int w, int h)
	: InteractiveRectangle(w,h)
	, DrawableRectangle(w,h)
	,mHovering(false)
	,mClicking(false)
//	,mCallback(NULL)
//	,mCallbackArgs(NULL)
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
		requestRedraw();
		return true;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
		bool lastHovering = mHovering;
		mHovering = isPointInside(event.mouse.x, event.mouse.y);
		if (mHovering != lastHovering ) {
			requestRedraw();
		}
		return mHovering;
	}
	else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
		bool inside = isPointInside(event.mouse.x, event.mouse.y);
		//if (inside && mClicking && mCallback) {
		//	mCallback(mCallbackArgs);
		//}
		if (inside && mClicking) {
			EventCallback::Details det;
			det.source = this;
			det.clickx = event.mouse.x;
			det.clicky = event.mouse.y;
			triggerEvent(EventCallback::clicked, det);
		}
		mClicking = false;
		mHovering = inside;
		requestRedraw();
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

bool jmg::Label::isEditing()
{
	return getContext().mWritingFocus == this;
}

void jmg::Label::setValue(const char * val)
{
	al_ustr_assign_cstr(mValue, val);
}

void jmg::Label::setValue(const char16_t * val)
{
	al_ustr_free(mValue);
	mValue = al_ustr_new_from_utf16((const uint16_t*)val);
}

jmg::Label::Label(const char * val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
	, mValue(al_ustr_new(val))
	, mFont(jmg::fetchDefaultFont())
	, mWidth(0xFFFFFF)
{
}

jmg::Label::Label(const char16_t * val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
	, mValue(al_ustr_new_from_utf16((uint16_t*)val))
	, mFont(jmg::fetchDefaultFont())
	, mWidth(0xFFFFFF)
{
}

jmg::Label::~Label()
{
	al_ustr_free(mValue);
}

jmg::Label::Label(const Label & other)
	: Base(other)
	, mValue(al_ustr_dup(other.mValue))
	, mFont(other.mFont)
	, mWidth(other.mWidth)
{
}

jmg::Label& jmg::Label::operator=(const Label & other)
{
	Base::operator=((const Base&)other);
	al_ustr_assign(mValue, other.mValue);
	mFont = other.mFont;
	mWidth = other.mWidth;
	return *this;
}

void jmg::Label::draw(int origx, int origy)
{
	//al_draw_text(font, color, origx + relx, origy + rely, 0, value.c_str());

	al_draw_multiline_ustr(mFont, mColor, origx + mRelx, origy + mRely, (float)mWidth,
		(float)al_get_font_line_height(mFont), 0, mValue);
}

int jmg::Label::getAsInt() const
{
	try {
		return std::stoi(std::string(al_cstr(mValue)));
	}
	catch (std::exception) {
		return 0;
	}
}

float jmg::Label::getAsFloat() const
{
	try {
		return std::stof(std::string(al_cstr(mValue)));
	}
	catch (std::exception) {
		return 0.0f;
	}
}

double jmg::Label::getAsDouble() const
{
	try {
		return std::stod(std::string(al_cstr(mValue)));
	}
	catch (std::exception) {
		return 0.0;
	}
}

int jmg::Label::getHeight() const
{
	//int bbx, bby, bbw, bbh;
	//al_get_ustr_dimensions(mFont, mValue, &bbx, &bby, &bbw, &bbh);
	//return bbh;
	return al_get_font_line_height(mFont);
}

void jmg::Text::insert(int keycode)
{
	al_ustr_insert_chr(mValue, al_ustr_offset(mValue, mTextPos), keycode);
	mSelectionPos = ++mTextPos;
	requestRedraw(1);
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
	: Base(0, 0, al_map_rgb(0, 0, 0))
	, Label(val)
	, cursorXRef(-2)
	, mTextPos(0)
	, mSelectionPos(0)
	, mClicking(false)
	, mIsNumeric(false)
{
}

jmg::Text::Text(const char16_t* val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
	, Label(val)
	, cursorXRef(-2)
	, mTextPos(0)
	, mSelectionPos(0)
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
	if (isEditing()) {
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
			Label*& writingFocus = getContext().mWritingFocus;
			if (writingFocus) {
				writingFocus->requestRedraw(-1); //notify focus lost
			}
			writingFocus = this;
			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				mClicking = true;
			}
			if (pos != mTextPos) {
				cursorXRef = event.mouse.x - mRelx - calcOrigX();
				mTextPos = pos;
				requestRedraw(1);
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
	else if (isEditing()) {
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
				requestRedraw(1);
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
				requestRedraw(1);
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
				int x, y;
				getCursorPosFromTextIndex(mTextPos, NULL, &y);
				x = cursorXRef + mRelx + calcOrigX();
				int pos = getTextIndexFromCursorPos(x, y - al_get_font_line_height(mFont));
				mTextPos = pos >= 0 ? pos : 0;
				requestRedraw(1);
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				int x, y;
				getCursorPosFromTextIndex(mTextPos, NULL, &y);
				x = cursorXRef + mRelx + calcOrigX();
				int pos = getTextIndexFromCursorPos(x, y + al_get_font_line_height(mFont));
				if (pos > 0) {
					mTextPos = pos;
					requestRedraw(1);
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
					requestRedraw(1);
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
				requestRedraw(1);
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
	getContext().mWritingFocus = nullptr;
	if (mIsNumeric) {
		if (al_ustr_size(mValue) == 0) {
			setFrom((double)0.0, mMaxDecimals);
		}
		else {
			const double val = getAsDouble();
			if (val > mMaxValue) {
				setFrom(mMaxValue, mMaxDecimals);
			}
			else if (val < mMinValue) {
				setFrom(mMinValue, mMaxDecimals);
			}
		}
	}
	//editHappened();
	triggerEvent(EventCallback::edited, { this });
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
	else if (isEditing()) {
		if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
			if (event.keyboard.keycode == ALLEGRO_KEY_A && event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) {
				mSelectionPos = 0;
				mTextPos = (int)al_ustr_length(mValue);
				resetCursorXRef();
				requestRedraw(1);
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
				if (collapseSelection()) {
					resetCursorXRef();
					requestRedraw(1);
				}
				else if (mTextPos > 0) {
					al_ustr_remove_chr(mValue, al_ustr_offset(mValue, --mTextPos));
					mSelectionPos = mTextPos;
					resetCursorXRef();
					requestRedraw(1);
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_DELETE) {
				if (collapseSelection()) {
					resetCursorXRef();
					requestRedraw(1);
				}
				else if (mTextPos < (int)al_ustr_length(mValue)) {
					al_ustr_remove_chr(mValue, al_ustr_offset(mValue, mTextPos));
					requestRedraw(1);
				}
				return true;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
				if (mIsNumeric) {
					confirmEditing();
					mSelectionPos = mTextPos;
					requestRedraw(1);
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
				getContext().mWritingFocus = nullptr;
				requestRedraw(1);
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
							doInsert = mMaxDecimals > 0 && pointPos < 0 && size - right <= mMaxDecimals && right - minusHere <= mMaxDigits;
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

void jmg::Numeric::init(double minValue, double maxValue, int maxDigits, int maxDecimals, bool positiveOnly)
{
	mIsNumeric = true;
	mMinValue = minValue;
	mMaxValue = maxValue;
	mMaxDigits = maxDigits;
	mMaxDecimals = maxDecimals;
	mPositiveOnly = positiveOnly;
}

jmg::Numeric::Numeric(char val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(-127, 127, 3, 0, false);
	setFrom(val,mMaxDecimals);
}

jmg::Numeric::Numeric(unsigned char val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(0, 255, 3, 0, true);
	setFrom(val, mMaxDecimals);
}

jmg::Numeric::Numeric(short val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(
		std::numeric_limits<short>::min(),
		std::numeric_limits<short>::max(),
		std::numeric_limits<short>::digits, 0, false);
	setFrom(val, mMaxDecimals);
}

jmg::Numeric::Numeric(unsigned short val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(
		std::numeric_limits<unsigned short>::min(),
		std::numeric_limits<unsigned short>::max(),
		std::numeric_limits<unsigned short>::digits, 0, true);
	setFrom(val, mMaxDecimals);
}

jmg::Numeric::Numeric(int val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max(),
		std::numeric_limits<int>::digits, 0, false);
	setFrom(val, mMaxDecimals);
}

jmg::Numeric::Numeric(unsigned int val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(
		std::numeric_limits<unsigned int>::min(),
		std::numeric_limits<unsigned int>::max(),
		std::numeric_limits<unsigned int>::digits, 0, true);
	setFrom(val, mMaxDecimals);
}

jmg::Numeric::Numeric(float val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(
		std::numeric_limits<float>::min(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::digits,
		std::numeric_limits<float>::digits10,
		false);
	setFrom(val, mMaxDecimals);
}

jmg::Numeric::Numeric(double val)
	: Base(0, 0, al_map_rgb(0, 0, 0))
{
	init(
		std::numeric_limits<double>::min(),
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::digits,
		std::numeric_limits<double>::digits10,
		false);
	setFrom(val, mMaxDecimals);
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
		case CHECK:
			img = al_create_bitmap(10, 10);
			al_set_target_bitmap(img);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			al_draw_line(1, 3, 4, 9, al_map_rgba(0, 0, 0, 255), 1.5f);
			al_draw_line(4, 9, 8, 1, al_map_rgba(0, 0, 0, 255), 1.5f);
			break;
		case PLUS:
			img = al_create_bitmap(10, 10);
			al_set_target_bitmap(img);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			al_draw_line(5, 1, 5, 8, al_map_rgba(0, 0, 0, 255), 1.0f);
			al_draw_line(1, 4, 8, 4, al_map_rgba(0, 0, 0, 255), 1.0f);
			break;
		case MINUS:
			img = al_create_bitmap(10, 10);
			al_set_target_bitmap(img);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			al_draw_line(1, 4, 8, 4, al_map_rgba(0, 0, 0, 255), 1.0f);
			break;
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

int jmg::Image::getHeight() const
{
	return mImage ? al_get_bitmap_height(mImage) : 0;
}

jmg::Context::Context()
	: mWritingFocus(nullptr)
	, mAutoAdd({ nullptr,0,0,0 })
{
}

void checkTheBox(const jmg::EventCallback::Details& details, void* arg) {
	jmg::CheckBox* checkBox = (jmg::CheckBox*)arg;
	if (checkBox->mChecked) {
		checkBox->mChecked = false;
		checkBox->mImage.remove();
	}
	else {
		checkBox->mChecked = true;
		checkBox->addChild(&checkBox->mImage);
	}
	//checkBox->editHappened();
	checkBox->triggerEvent(jmg::EventCallback::edited, { checkBox });
}

jmg::CheckBox::CheckBox(bool startsChecked)
	: InteractiveRectangle(10,10)
	, Button(10,10)
	, mImage(jmg::Image::CHECK)
	, mChecked(!startsChecked)
{
	checkTheBox({ nullptr }, (void*)this);
	//mCallback = checkTheBox;
	//mCallbackArgs = (void*)this;
	subscribeToEvent(EventCallback::clicked, { checkTheBox, this });
}

void jmg::CheckBox::draw(int origx, int origy)
{
	jmg::Button::draw(origx, origy);
}

void showHideCallback(const jmg::EventCallback::Details& details, void* arg) {
	jmg::ShowHide* sh = (jmg::ShowHide*)arg;

	sh->toggle();
}

jmg::ShowHide::ShowHide()
	: InteractiveRectangle(10, 10)
	, mRememberParent(nullptr)
	, mShowHideObject(nullptr)
	, mShowHideHeight(0)
	, mPlusMinus(jmg::Image::MINUS)
{
	addChild(&mPlusMinus);
	subscribeToEvent(EventCallback::clicked, { showHideCallback, this });
}

jmg::ShowHide::~ShowHide()
{
	setShowHideObject(nullptr);
}

void jmg::ShowHide::addRemCallback(const jmg::EventCallback::Details & details, void * arg) {
	jmg::ShowHide* sh = (jmg::ShowHide*)arg;

	if (sh->mShowHideObject) {
		int newHeight = sh->mShowHideObject->getHeight();
		sh->shiftSiblings(newHeight - sh->mShowHideHeight);
		sh->mShowHideHeight = newHeight;
		sh->requestRedraw(-1);
	}
}

void jmg::ShowHide::setShowHideObject(Base * obj)
{
	mRememberParent = nullptr;
	EventCallback callback = { addRemCallback, this};
	if (mShowHideObject) {
		mShowHideObject->unsubscribeToEvent(EventCallback::childAdded, callback);
		mShowHideObject->unsubscribeToEvent(EventCallback::childRemoved, callback);
	}
	mShowHideObject = obj;
	if (mShowHideObject) {
		mShowHideObject->subscribeToEvent(EventCallback::childAdded, callback);
		mShowHideObject->subscribeToEvent(EventCallback::childRemoved, callback);
		mShowHideHeight = mShowHideObject->getHeight();
	}
	else {
		mShowHideHeight = 0;
	}
}

void jmg::ShowHide::show()
{
	if (mShowHideObject && mRememberParent) {
		mRememberParent->addChild(mShowHideObject);
		// push siblings down
		mShowHideHeight = mShowHideObject->getHeight();
		shiftSiblings(mShowHideHeight);

		mPlusMinus.mImage = jmg::Image::getImage(jmg::Image::MINUS);
		requestRedraw(-1);
	}
}

void jmg::ShowHide::hide()
{
	if (mShowHideObject) {
		// pull siblings up
		shiftSiblings(-mShowHideHeight);

		mRememberParent = mShowHideObject->parent();
		mShowHideObject->remove();
		mPlusMinus.mImage = jmg::Image::getImage(jmg::Image::PLUS);
		requestRedraw(-1);
	}
}

void jmg::ShowHide::toggle()
{
	if (mShowHideObject->parent()) {
		hide();
	}
	else {
		show();
	}
}

void jmg::ShowHide::shiftSiblings(int amount)
{
	if (amount != 0 && mShowHideObject->parent()) {
		for (auto sibling : mShowHideObject->parent()->children()) {
			if (sibling->mRely >= mShowHideObject->mRely && sibling != this && sibling != mShowHideObject) {
				sibling->mRely += amount;
			}
		}
	}
}

void addChildCallback(const jmg::EventCallback::Details& details, void* arg) {
	jmg::Cropper* crp = (jmg::Cropper*)arg;
	if (details.addedChild && details.addedChild != &crp->mRoot) {
		crp->mRoot.addChild(details.addedChild);
	}
}

jmg::Cropper::Cropper(int w, int h)
	: DrawableRectangle(w, h)
	, InteractiveRectangle(w, h)
	, mRender(nullptr)
{
	mRoot.mButton = 3;
	subscribeToEvent(EventCallback::childAdded, { addChildCallback, this });
}

jmg::Cropper::~Cropper()
{
	if (mRender) {
		al_destroy_bitmap(mRender);
		mRender = nullptr;
	}
}

void jmg::Cropper::draw(int origx, int origy)
{
	const int bw = mWidth + 1, bh = mHeight + 1;
	if (mRender && (al_get_bitmap_width(mRender) != bw || al_get_bitmap_height(mRender) != bh)) {
		al_destroy_bitmap(mRender);
		mRender = nullptr;
	}
	if (!mRender) {
		mRender = al_create_bitmap(bw, bh);
	}

	ALLEGRO_BITMAP* target = al_get_target_bitmap();
	al_set_target_bitmap(mRender);
	if (mRoot.needsRedraw()) {
		DrawableRectangle::draw(-mRelx + 1, -mRely);
	}

	mRoot.mRelx += 1;
	mRoot.baseDraw();
	mRoot.mRelx -= 1;

	al_set_target_bitmap(target);

	al_draw_bitmap(mRender, origx + mRelx - 1, origy + mRely - 1, 0);
}

bool jmg::Cropper::handleEvent(const ALLEGRO_EVENT & event)
{
	bool doRootEvent = false;
	if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
	 || event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
		if (catchMouse(event, -1, event.type)) {
			doRootEvent = true;
		}
	}
	else {
		doRootEvent = true;
	}

	bool ret = false;
	if (doRootEvent) {
		mRoot.mRelx += mRelx; mRoot.mRely += mRely;
		ret = mRoot.baseHandleEvent(event);
		mRoot.mRelx -= mRelx; mRoot.mRely -= mRely;
	}
	if (mRoot.needsRedraw()) {
		requestRedraw(); // TODO : should be called if ANY root child needs redraw
	}
	return ret;
}

