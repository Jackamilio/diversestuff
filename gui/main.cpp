#include "Engine.h"
#include <allegro5/allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include "JackamikazGUI.h"

class JmGui : public Engine::Graphic, public Engine::Input {
public:
	Engine & engine;

	jmg::Base root;
	jmg::WallPaper wp;
	jmg::Window win;
	jmg::Text text;
	jmg::MoveableRectangle mr;

	JmGui(Engine& e)
		: engine(e)
		, wp(al_map_rgb(200, 200, 200))
		, win(200, 250, "Salut les gens")
		, text(u"Salut je teste ma vie\ngenre lol ‡‡‡‡‡‡ÈÈÈÈÈÈ ouais trop bien tavu ouech genre vazi quoi")
		, mr(200, 350)
	{
		engine.overlayGraphic.AddChild(this);
		engine.inputRoot.AddChild(this);

		mr.addChild(&text);
		root.addChild(&mr);

		win.mColor.g = 0;
		win.mOutline = 1;
		win.mRelx = 300;
		win.mRely = 150;

		text.mRelx = 10;
		text.mRely = 10;
		text.mWidth = mr.mWidth - text.mRelx * 2;

		win.setParent(&root, false);
	}

	void Draw() {
		root.needsRedraw();
		root.baseDraw();
	}

	void Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_KEY_UP
		 && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			win.open();
		}
		root.baseHandleEvent(event);
	}
};

class FPSCounter : public Engine::Graphic {
public:
	Engine & engine;

	double samplingDuration;
	double lastTime;
	int nbSamples;
	int lastFps;

	FPSCounter(Engine& e) : engine(e) {
		engine.overlayGraphic.AddChild(this);

		samplingDuration = 0.5;
		lastTime = engine.time;
		nbSamples = 0;
		lastFps = (int)(1.0 / engine.dtTarget);
	}

	void Draw() {

		++nbSamples;
		if (engine.time - lastTime > samplingDuration) {
			lastFps = (int)((double)nbSamples / samplingDuration);
			nbSamples = 0;
			lastTime = engine.time;
		}

		al_draw_textf(jmg::fetchDefaultFont(), al_map_rgb(255, 255, 255), 10, 10, 0, "FPS : %i", lastFps);
	}
};

int main(int nbarg, char ** args) {
	Engine engine;

	if (engine.Init()) {
		al_init_primitives_addon();
		al_init_ttf_addon();

		FPSCounter fc(engine);
		JmGui gui(engine);

		while (engine.OneLoop()) {}
	}
}

/*
#include <iostream>
#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\display.h>
#include <allegro5\events.h>
#include "JackamikazGUI.h"

int main(int nbarg, char ** args)
{

	al_init();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	ALLEGRO_DISPLAY* display = al_create_display(640, 480);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_install_mouse();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_install_keyboard();
	al_register_event_source(queue, al_get_keyboard_event_source());

	ALLEGRO_EVENT event;

	jmg::WallPaper wp(al_map_rgb(200, 200, 200));
	jmg::Window win(200,250,"Salut les gens");
	jmg::Text text(u"Salut je teste ma vie\ngenre lol ‡‡‡‡‡‡ÈÈÈÈÈÈ ouais trop bien tavu ouech genre vazi quoi");
	jmg::MoveableRectangle mr(200,350);
	mr.addChild(&text);
	wp.addChild(&mr);

	win.mColor.g = 0;
	win.mOutline = 1;
	win.mRelx = 300;
	win.mRely = 150;

	text.mRelx = 10;
	text.mRely = 10;
	text.mWidth = mr.mWidth - text.mRelx * 2;

	win.setParent(&wp, false);


	bool quitApp = false;
	while (!quitApp)
	{
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			quitApp = true;
		}
		else {
			if (event.type == ALLEGRO_EVENT_KEY_UP
			 && event.keyboard.keycode == ALLEGRO_KEY_F1) {
				win.open();
			}
			wp.baseHandleEvent(event);
			wp.baseDraw();
			al_flip_display();
		}
	}

	al_destroy_font(jmg::fetchDefaultFont());
	al_destroy_display(display);

	al_shutdown_primitives_addon();

	return 0;
}
*/
