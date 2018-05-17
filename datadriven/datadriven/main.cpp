#include <allegro5/allegro.h>
#include "exposing.h"
#include <AntTweakBar.h>
#include <iostream>
#include "Tests.h"

using namespace std;

// Function called by AntTweakBar to copy the content of a std::string handled
// by the AntTweakBar library to a std::string handled by your application
void TW_CALL CopyStdStringToClient(string& destinationClientString, const string& sourceLibraryString)
{
	destinationClientString = sourceLibraryString;
}

const float FPS = 60.0f;

class RVB {
public:
	union {
		struct {
			unsigned char r;
			unsigned char v;
			unsigned char b;
			unsigned char a;
		};
		unsigned int color = 0;
	};

	IM_AN_EXPOSER
};

#define EXPOSE_TYPE RVB
EXPOSE_START
EXPOSE(r)
EXPOSE(v)
EXPOSE(b)
EXPOSE_AS(COLOR32, color)
EXPOSE_END
#undef EXPOSE_TYPE

int main(int nbarg, void** args) {
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;

	bool running = true;
	bool redraw = true;

	// Initialize allegro
	if (!al_init()) {
		cerr << "Failed to initialize allegro.\n" << endl;
		return 1;
	}

	// Initialize the timer
	timer = al_create_timer(1.0 / FPS);
	if (!timer) {
		cerr << "Failed to create timer.\n" << endl;
		return 1;
	}

	// Create the display
	al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
	int wwidth = 640;
	int wheight = 480;
	display = al_create_display(wwidth, wheight);
	if (!display) {
		cerr << "Failed to create display.\n" << endl;
		return 1;
	}


	// AntTweakBar time, baby!
	TwInit(TW_OPENGL, NULL);
	TwCopyStdStringToClientFunc(CopyStdStringToClient);
	TwWindowSize(wwidth, wheight);

	exposureBar = TwNewBar("Exposure bar");

	RVB rvb;
	EXPOSE_GLOBAL(rvb);
	

	// Keyboard and mouse
	al_install_mouse();
	al_install_keyboard();

	// Create the event queue
	event_queue = al_create_event_queue();
	if (!event_queue) {
		cerr << "Failed to create event queue." << endl;
		return 1;
	}

	// Register event sources
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	// Display a black screen
	al_clear_to_color(al_map_rgb(rvb.r,rvb.v,rvb.b));
	al_flip_display();

	// Start the timer
	al_start_timer(timer);

	// Game loop
	while (running) {
		ALLEGRO_EVENT event;
		ALLEGRO_TIMEOUT timeout;

		// Initialize timeout
		al_init_timeout(&timeout, 0.06);

		// Fetch the event (if one exists)
		bool get_event = al_wait_for_event_until(event_queue, &event, &timeout);

		// Handle the event
		if (get_event) {
			switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				redraw = true;
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				running = false;
				break;
			case ALLEGRO_EVENT_KEY_CHAR: {
				int mod = 0;
				if (event.keyboard.modifiers & ALLEGRO_KEYMOD_SHIFT)
					mod |= TW_KMOD_SHIFT;
				if (event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL)
					mod |= TW_KMOD_CTRL;
				if (event.keyboard.modifiers & ALLEGRO_KEYMOD_ALT)
					mod |= TW_KMOD_ALT;
				if (event.keyboard.unichar) {
					TwKeyPressed(event.keyboard.unichar, mod);
				}
				else {
					int code;
					switch (event.keyboard.keycode) {
					case ALLEGRO_KEY_BACKSPACE:	code = TW_KEY_BACKSPACE; break;
					case ALLEGRO_KEY_TAB:		code = TW_KEY_TAB; break;
					//case ALLEGRO_KEY_???:		code = TW_KEY_CLEAR; break; I have NO idea what is a "clear" key on the keyboard. The name doesn't help with research too.
					case ALLEGRO_KEY_ENTER:		code = TW_KEY_RETURN; break;
					case ALLEGRO_KEY_PAUSE:		code = TW_KEY_PAUSE; break;
					case ALLEGRO_KEY_ESCAPE:	code = TW_KEY_ESCAPE; break;
					case ALLEGRO_KEY_SPACE:		code = TW_KEY_SPACE; break;
					case ALLEGRO_KEY_DELETE:	code = TW_KEY_DELETE; break;
					case ALLEGRO_KEY_UP:		code = TW_KEY_UP; break;
					case ALLEGRO_KEY_DOWN:		code = TW_KEY_DOWN; break;
					case ALLEGRO_KEY_RIGHT:		code = TW_KEY_RIGHT; break;
					case ALLEGRO_KEY_LEFT:		code = TW_KEY_LEFT; break;
					case ALLEGRO_KEY_INSERT:	code = TW_KEY_INSERT; break;
					case ALLEGRO_KEY_HOME:		code = TW_KEY_HOME; break;
					case ALLEGRO_KEY_END:		code = TW_KEY_END; break;
					case ALLEGRO_KEY_PGUP:		code = TW_KEY_PAGE_UP; break;
					case ALLEGRO_KEY_PGDN:		code = TW_KEY_PAGE_DOWN; break;
					default: code = 0; break;
					}
					if (event.keyboard.keycode >= ALLEGRO_KEY_F1 && event.keyboard.keycode <= ALLEGRO_KEY_F12) {
						code = TW_KEY_F1 + event.keyboard.keycode - ALLEGRO_KEY_F1;
					}
					TwKeyPressed(code, mod);
				}
			}
				break;
			case ALLEGRO_EVENT_MOUSE_AXES:
				TwMouseMotion(event.mouse.x, event.mouse.y);
				TwMouseWheel(event.mouse.z);
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
				TwMouseButton(TwMouseAction::TW_MOUSE_PRESSED, (TwMouseButtonID)event.mouse.button);
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
				TwMouseButton(TwMouseAction::TW_MOUSE_RELEASED, (TwMouseButtonID)event.mouse.button);
				break;
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(display);
				wwidth = event.display.width;
				wheight = event.display.height;
				TwWindowSize(wwidth, wheight);
				break;
			default:
				//cerr << "Unsupported event received: " << event.type << endl;
				break;
			}
		}

		// Check if we need to redraw
		if (redraw && al_is_event_queue_empty(event_queue)) {
			// Redraw
			al_clear_to_color(al_map_rgb(rvb.r, rvb.v, rvb.b));

			TwDraw();

			al_flip_display();
			redraw = false;
		}
	}

	// Clean up
	TwTerminate();
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}