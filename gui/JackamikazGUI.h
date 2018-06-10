#ifndef __JACKAMIKAZ_GUI_H__
#define __JACKAMIKAZ_GUI_H__

#include <allegro5\allegro.h>
#include <allegro5\allegro_font.h>
#include <allegro5\events.h>
#include <allegro5\utf8.h>
#include <list>
#include <vector>
#include <string>

#pragma warning(disable:4250)

namespace jmg
{
	class Base;

	struct EventCallback {
		enum Type {
			clicked, edited, added, removed, childAdded, childRemoved

			, size
		};
		struct Details {
			Base* source;
			union {
				Base* addedFrom;
				Base* removedFrom;
				Base* addedChild;
				Base* removedChild;
				struct {
					int clickx;
					int clicky;
				};
			};
		};
		void(*mFunction)(const Details& details,void*);
		void* mArgs;

		inline bool operator==(const EventCallback& rhs) { return mFunction == rhs.mFunction; }
	};

	class Context;
	
	class Base {
	public:
		typedef std::list<Base*> Children;
	private:
		void redraw(int origx, int origy);
		void cascadeDraw(int origx, int origy, bool parentNeedsIt = false);
		bool cascadeHandleEvent(const ALLEGRO_EVENT& event);

		Base * mParent;
		Children mChildren;
		bool mNeedsRedraw;

		bool mRemoveMe;
		bool mDeleteMe;

		std::list<EventCallback> mEventSubs[EventCallback::size];
	protected:
		static Context& getContext();
		virtual void draw(int origx, int origy);
		virtual bool handleEvent(const ALLEGRO_EVENT& event);

		//virtual void onAddChild(Base* child) {}
		//virtual void onRemoveChild(Base* child) {}
	public:
		int mRelx;
		int mRely;
		ALLEGRO_COLOR mColor;

		Base(int relx=0, int rely=0, ALLEGRO_COLOR color=al_map_rgb(255,255,255));
		virtual ~Base();

		inline Base* parent() { return mParent; }
		inline const Children& children() { return mChildren; }

		bool has(const Base* child) const;

		enum Edge { TOP, BOTTOM, LEFT, RIGHT };
		void addChild(Base* child);
		void addChild(Base* child, int relx, int rely);
		void addChild(Base* child, Edge pos, int xory);
		int getEdge(Edge edge) const;

		void setAsAutoAddRef(int startx = 0, int starty = 0, int additionalMargin = 0);
		void autoAdd(Base* parent = nullptr);
		static void autoAddShift(int shiftx, int shifty);

		void remove(bool del = false);

		void baseDraw();
		bool baseHandleEvent(const ALLEGRO_EVENT& event);

		int calcOrigX() const;
		int calcOrigY() const;

		virtual int getHeight() const = 0;

		inline bool needsRedraw() const { return mNeedsRedraw; }
		void requestRedraw(int depth = 0);

		void subscribeToEvent(EventCallback::Type type, EventCallback callback);
		void unsubscribeToEvent(EventCallback::Type type, EventCallback callback);
		void triggerEvent(EventCallback::Type type, const EventCallback::Details& details);
	};

	class Root : public virtual Base {
	public:
		int getHeight() const { return 0; }
	};

	class Container : public virtual Base {
	public:
		int getHeight() const { return getEdge(BOTTOM); }
	};

	//class WallPaper : public virtual Base{
	//public:
	//	WallPaper(const ALLEGRO_COLOR& color);
	//	void draw(int, int);
	//};

	class Image : public virtual Base {
	public:
		enum PreRenderedImage {
			CROSS,
			CHECK,
			RADIO,
			PLUS,
			MINUS,
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
		int getHeight() const;
	};

	//class Editable {
	//public:
	//	void(*mEditCallback)(void*);
	//	void* mEditCallbackArgs;

	//	Editable();
	//	void editHappened();
	//};

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

		Label(const Label& other);
		Label& operator= (const Label& other);

		void draw(int origx, int origy);

		int getAsInt() const;
		float getAsFloat() const;
		double getAsDouble() const;

		template<typename T>
		void setFrom(T val, int maxDecimals = 999999);

		int getHeight() const;
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

	class Text : public Label {//, public Editable {
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
		bool isPointInside(int px, int py) const;
		bool catchMouse(const ALLEGRO_EVENT& event, int button = 1, ALLEGRO_EVENT_TYPE evType = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) const;
		void addAndAdaptLabel(Label* label, int leftMargin = 0, int topMargin = -1, int rightMargin = -1); // -1 means : copy leftMargin
		int getHeight() const;
	};

	class Moveable : public virtual Base {
	private:
		bool mHeld;
		int mDx;
		int mDy;
	public:
		Base * mTarget;
		unsigned int mButton;

		virtual bool catchCondition(const ALLEGRO_EVENT& event) const;

		Moveable();
		bool handleEvent(const ALLEGRO_EVENT& event);

		int getHeight() const;
	};

	class MoveableRectangle : public Moveable, public virtual InteractiveRectangle {
	public:
		bool catchCondition(const ALLEGRO_EVENT& event) const;

		MoveableRectangle(int w, int h);

		int getHeight() const;
	};

	class DrawableRectangle : public virtual InteractiveRectangle {
	public:
		DrawableRectangle(int w, int h);
		void draw(int origx, int origy);

		unsigned char mOutline;
	};

	class MoveableDrawableRectangle : public MoveableRectangle, public DrawableRectangle {
	public:
		MoveableDrawableRectangle(int w,int h);
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

		//void(*mCallback)(void*);
		//void* mCallbackArgs;
	};

	class CheckBox : public Button {//, public Editable {
	public:
		bool mChecked;
		Image mImage;

		CheckBox(bool startsChecked = false);
		
		void draw(int origx, int origy);
	};

	class ShowHide : public Button {
	private:
		Base * mRememberParent;
		Base* mShowHideObject;
		int mShowHideHeight;

		static void addRemCallback(const EventCallback::Details& details, void* arg);
	public:
		Image mPlusMinus;

		ShowHide();
		~ShowHide();

		void setShowHideObject(Base* obj);
		void show();
		void hide();
		void toggle();
		void shiftSiblings(int amount);
		
	};

	class Cropper : public virtual DrawableRectangle {
	private:
		ALLEGRO_BITMAP * mRender;

	public:
		Moveable mRoot;

		Cropper(int w, int h);
		~Cropper();

		void draw(int origx, int origy);
		bool handleEvent(const ALLEGRO_EVENT& event);

		//void onAddChild(Base* child);
	};

	class Window : public DrawableRectangle {
	private:
		Base * mContext;
	public:
		MoveableDrawableRectangle mMover;
		Button mBtnClose;
		Image mBtnImage;
		Label mCaption;

		Window(int w, int h, const char* caption = "Window");
		bool handleEvent(const ALLEGRO_EVENT& event);

		void open();
		void close();

		void setContext(Base* context, bool startsOpen);
	};

	class Context {
	public:
		Label * mWritingFocus;

		struct AutoAdd {
			Base* mReference;
			int mRelx;
			int mRely;
			int mAddititonalMargin;
		};
		AutoAdd mAutoAdd;

		Context();
	};

	ALLEGRO_FONT* fetchDefaultFont();
}

#endif
