#ifndef __APP_TEST_H__
#define __APP_TEST_H__

#include <allegro5/allegro.h>
#include "Engine.h"
#include "gui.h"

void* fltk_thread(ALLEGRO_THREAD* t, void* arg);

class AppTest
{
public:
	AppTest();
	~AppTest();

private:
	Engine* engine;
	ALLEGRO_THREAD* fltkThread;
	bool init_success;

	void run();
};

#endif
