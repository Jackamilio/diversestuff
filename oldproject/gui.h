#ifndef ___GUI_H___
#define ___GUI_H___

class Fl_Window;

namespace Gui {
	struct GuiInternal;
	struct GuiContext {
		GuiInternal* internal;
		Fl_Window* mainWindow;
	};

	namespace Message {
		enum Enum {
			none = 0,
			openMainWindow = 1,
			quit
		};
	}

	GuiContext* Load();
	void Unload(GuiContext*);

	void Send(Message::Enum msg);
	bool HandleMessage(GuiContext*, void*);
	int SetupAndLaunch();
}

#endif
