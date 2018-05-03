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

	jmg::WallPaper wp(al_map_rgb(255, 255, 255));
	jmg::Window win(200,250,"Salut les gens");
	jmg::MoveableRectangle mr(100, 100);
	jmg::Text text("Salut je teste ma vie genre lol ouais trop bien\n tavu ouech genre vazi quoi");
	mr.addChild(&text);

	win.color.g = 0;
	win.outline = 1;

	mr.addChild(&win);
	wp.addChild(&mr);


	bool quitApp = false;
	while (!quitApp)
	{
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			quitApp = true;
		}
		else {
			wp.baseHandleEvent(event);
			wp.baseDraw();
			al_flip_display();
		}
	}

	al_destroy_font(jmg::FetchDefaultFont());
	al_destroy_display(display);

	al_shutdown_primitives_addon();

	return 0;
}