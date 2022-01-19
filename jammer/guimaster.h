#ifndef __GUI_MASTER_H__
#define __GUI_MASTER_H__

#include <allegro5/allegro5.h>

class GuiMaster;

class GuiElement {
public:
	virtual void draw(GuiMaster& master);
	virtual void processEvent(ALLEGRO_EVENT& evt);
};

class GuiMaster {
public:

};



#endif //__GUI_MASTER_H__