#ifndef __JACKAMIKAZ_GUI_H__
#define __JACKAMIKAZ_GUI_H__

#include <allegro5\allegro.h>
#include <allegro5\allegro_font.h>
#include <allegro5\events.h>
#include <vector>
#include <string>

#pragma warning(disable:4250)

namespace jmg
{
	class Base {
	private:
		void redraw(int origx, int origy);
		void cascadeDraw(int origx, int origy, bool parentNeedsIt = false);
		bool cascadeHandleEvent(const ALLEGRO_EVENT& event);

		Base * parent;
		std::vector<Base*> children;

	protected:
		virtual void draw(int origx, int origy);
		virtual bool handleEvent(const ALLEGRO_EVENT& event);

		bool needsRedraw;
	public:
		int relx;
		int rely;

		Base();

		void addChild(Base* child);
		void remove();

		void baseDraw();
		void baseHandleEvent(const ALLEGRO_EVENT& event);

		int calcOrigX() const;
		int calcOrigY() const;

		void moved();
	};

	class WallPaper : public Base{
	public:
		ALLEGRO_COLOR color;
		WallPaper(const ALLEGRO_COLOR& color);
		void draw(int, int);
	};

	class Rectangle {
	public:
		Rectangle();
		Rectangle(int w, int h);
		int width;
		int height;
	};

	class InteractiveRectangle : public virtual Rectangle, public virtual Base {
	public:
		InteractiveRectangle();
		InteractiveRectangle(int w, int h);
		bool isPointInside(int px, int py);
		bool catchMouse(const ALLEGRO_EVENT& event, int button = 1, ALLEGRO_EVENT_TYPE evType = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
	};

	class Moveable : public InteractiveRectangle {
	private:
		bool held;
		int dx;
		int dy;
	public:
		Base * target;
		unsigned int button;

		Moveable();
		Moveable(int w, int h);
		bool handleEvent(const ALLEGRO_EVENT& event);
	};

	class DrawableRectangle : public virtual InteractiveRectangle {
	public:
		DrawableRectangle();
		DrawableRectangle(int w, int h);
		void draw(int origx, int origy);

		ALLEGRO_COLOR color;
		unsigned char outline;
	};

	class MoveableRectangle : public Moveable, public DrawableRectangle {
	public:
		MoveableRectangle(int w,int h);
	};

	class Button : public DrawableRectangle {
	private:
		bool hovering;
		bool clicking;

	public:
		Button();
		Button(int w, int h);

		void draw(int origx, int origy);
		bool handleEvent(const ALLEGRO_EVENT& event);

		void(*callback)(void*);
		void* args;
	};

	class Label : public Base {
	public:
		std::string value;
		ALLEGRO_COLOR color;
		ALLEGRO_FONT* font;
		Label(const char* val = "");

		void draw(int origx, int origy);
	};

	class Text : public Label {
	private:
		int tx;
		int ty;

		int textPos;
	public:
		Text(const char* val = "");

		void draw(int origx, int origy);
		bool handleEvent(const ALLEGRO_EVENT& event);
	};

	class Window : public DrawableRectangle {
	private:
		MoveableRectangle mover;
		Button btnClose;
		Label caption;
	public:
		Window(int w, int h, const char* caption = "Window");
		bool handleEvent(const ALLEGRO_EVENT& event);

		void close();
	};

	ALLEGRO_FONT* FetchDefaultFont();
}

#endif
