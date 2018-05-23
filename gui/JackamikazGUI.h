#ifndef __JACKAMIKAZ_GUI_H__
#define __JACKAMIKAZ_GUI_H__

#include <allegro5\allegro.h>
#include <allegro5\allegro_font.h>
#include <allegro5\events.h>
#include <allegro5\utf8.h>
#include <list>
#include <string>

#pragma warning(disable:4250)

namespace jmg
{
	class Context;
	
	class Base {
	private:
		Context * mContext;
		bool mIsContextOwner;

		void redraw(int origx, int origy);
		void cascadeDraw(int origx, int origy, bool parentNeedsIt = false);
		bool cascadeHandleEvent(const ALLEGRO_EVENT& event);

		Base * mParent;
		std::list<Base*> mChildren;
		bool mNeedsRedraw;

		bool mRemoveMe;
	protected:
		Context& getContext();
		virtual void draw(int origx, int origy);
		virtual bool handleEvent(const ALLEGRO_EVENT& event);
	public:
		int mRelx;
		int mRely;
		ALLEGRO_COLOR mColor;
		bool mDeleteMe;

		Base(int relx=0, int rely=0, ALLEGRO_COLOR color=al_map_rgb(255,255,255));
		virtual ~Base();

		inline Base* parent() { return mParent; }

		bool has(const Base* child) const;

		void addChild(Base* child);
		void remove();

		void baseDraw();
		bool baseHandleEvent(const ALLEGRO_EVENT& event);

		int calcOrigX() const;
		int calcOrigY() const;

		void needsRedraw(int depth = 0);
	};

	class WallPaper : public virtual Base{
	public:
		WallPaper(const ALLEGRO_COLOR& color);
		void draw(int, int);
	};

	class Image : public virtual Base {
	public:
		enum PreRenderedImage {
			CROSS,
			CHECK,
			RADIO,
			ARROW_UP,
			ARROW_DOWN,
			ARROW_LEFT,
			ARROW_RIGHT
		};

		static ALLEGRO_BITMAP* getImage(PreRenderedImage img);

		ALLEGRO_BITMAP* mImage;

		Image(const ALLEGRO_COLOR& color = al_map_rgb(255,255,255));
		Image(const char* file, const ALLEGRO_COLOR& color = al_map_rgb(255, 255, 255));
		Image(ALLEGRO_BITMAP* bitmap, const ALLEGRO_COLOR& color = al_map_rgb(255, 255, 255));
		Image(PreRenderedImage image, const ALLEGRO_COLOR& color = al_map_rgb(255, 255, 255));

		void draw(int, int);
	};

	class Label : public virtual Base {
	protected:
		ALLEGRO_USTR * mValue;

	public:
		ALLEGRO_FONT * mFont;
		int mWidth;

		bool isEditing();
		inline int getCharAt(int pos) const { return al_ustr_get(mValue, al_ustr_offset(mValue, pos)); }

		inline const char* getValue() const { return al_cstr(mValue); }
		void setValue(const char* val);
		void setValue(const char16_t* val);

		Label(const char* val = "");
		Label(const char16_t* val);
		~Label();

		void draw(int origx, int origy);

		int getAsInt() const;
		float getAsFloat() const;
		double getAsDouble() const;

		template<typename T>
		void setFrom(T val, int maxDecimals = 999999);
	};

	template<typename T>
	inline void Label::setFrom(T val, int maxDecimals)
	{
		setValue(std::to_string(val).c_str());
		const int pointPos = al_ustr_find_chr(mValue, 0, '.');
		const int size = (int)al_ustr_size(mValue);
		if (pointPos >= 0 && size - pointPos > maxDecimals) {
			al_ustr_remove_range(mValue, pointPos, size);
		}
	}

	class Text : public Label {
	private:
		void insert(int keycode);
		int getTextIndexFromCursorPos(int fromx, int fromy) const;
		void getCursorPosFromTextIndex(int pos, int* posx, int* posy) const;

		int cursorXRef;
		void resetCursorXRef();

		bool mClicking;

		bool handleCursorPosEvents(const ALLEGRO_EVENT& event);
		bool collapseSelection();
		void confirmEditing();
	protected:
		bool mIsNumeric;
	public:
		int mTextPos;
		int mSelectionPos;

		void(*mEditCallback)(void*);
		void* mEditCallbackArgs;

		double mMinValue;
		double mMaxValue;
		int mMaxDigits;
		int mMaxDecimals;
		bool mPositiveOnly;

		Text(const char* val = "");
		Text(const char16_t* val);

		void draw(int origx, int origy);
		bool handleEvent(const ALLEGRO_EVENT& event);
	};

	class Numeric : public Text {
	public:
		void init(double minValue, double maxValue, int maxDigits, int maxDecimals, bool positiveOnly);

		Numeric(char val);
		Numeric(unsigned char val);
		Numeric(short val);
		Numeric(unsigned short val);
		Numeric(int val);
		Numeric(unsigned int val);
		Numeric(float val);
		Numeric(double val);
	};

	class Rectangle {
	public:
		Rectangle(int w, int h);
		int mWidth;
		int mHeight;
	};

	class InteractiveRectangle : public Rectangle, public virtual Base {
	public:
		InteractiveRectangle(int w, int h);
		bool isPointInside(int px, int py);
		bool catchMouse(const ALLEGRO_EVENT& event, int button = 1, ALLEGRO_EVENT_TYPE evType = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
		void addAndAdaptLabel(Label* label, int leftMargin = 0, int topMargin = -1, int rightMargin = -1); // -1 means : copy leftMargin
	};

	class Moveable : public virtual InteractiveRectangle {
	private:
		bool mHeld;
		int mDx;
		int mDy;
	public:
		Base * mTarget;
		unsigned int mButton;

		Moveable(int w, int h);
		bool handleEvent(const ALLEGRO_EVENT& event);
	};

	class DrawableRectangle : public virtual InteractiveRectangle {
	public:
		DrawableRectangle(int w, int h);
		void draw(int origx, int origy);

		unsigned char mOutline;
	};

	class MoveableRectangle : public Moveable, public DrawableRectangle {
	public:
		MoveableRectangle(int w,int h);
	};

	class Button : public DrawableRectangle {
	private:
		bool mHovering;
		bool mClicking;

	public:
		Button();
		Button(int w, int h);

		void draw(int origx, int origy);
		bool handleEvent(const ALLEGRO_EVENT& event);

		void(*mCallback)(void*);
		void* mCallbackArgs;
	};

	class CheckBox : public Button, public Image {
	public:
		bool mChecked;

		CheckBox();
		
		void draw(int origx, int origy);
	};

	class Window : public DrawableRectangle {
	public:
		MoveableRectangle mMover;
		Button mBtnClose;
		Image mBtnImage;
		Label mCaption;

		Window(int w, int h, const char* caption = "Window");
		bool handleEvent(const ALLEGRO_EVENT& event);

		void open();
		void close();

		void setParent(Base* parent, bool startsOpen);
	};

	class Context {
	public:
		Label * mWritingFocus;

		Context();
	};

	ALLEGRO_FONT* fetchDefaultFont();
}

#endif
