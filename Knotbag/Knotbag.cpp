#include <iostream>
#include <allegro5/allegro5.h>
#include "imgui_impl_allegro5.h"
#include <allegro5/allegro_primitives.h>
#include "lua/lua.hpp"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

namespace DearImguiIntegration {
	void Init(ALLEGRO_DISPLAY* display) {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplAllegro5_Init(display);
	}

	bool Event(ALLEGRO_EVENT& event) {
		ImGui_ImplAllegro5_ProcessEvent(&event);

		if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			//al_acknowledge_resize(display);
			ImGui_ImplAllegro5_CreateDeviceObjects();
		}

		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureKeyboard || io.WantCaptureMouse;
	}

	void NewFrame() {
		// Start the Dear ImGui frame
		ImGui_ImplAllegro5_NewFrame();
		ImGui::NewFrame();
	}

	void Render() {
		ImGui::Render();
		ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
	}

	void Destroy() {
		ImGui_ImplAllegro5_Shutdown();
		ImGui::DestroyContext();
	}
}

int main()
{
	// Init allegro
	if (!al_install_system(ALLEGRO_VERSION_INT, nullptr)) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return false;
	}

	if (!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return false;
	}

	if (!al_install_mouse()) {
		fprintf(stderr, "failed to install mouse!\n");
		return false;
	}

	if (!al_init_primitives_addon()) {
		fprintf(stderr, "failed to load primitives addon!\n");
		return false;
	}

//	if (!al_init_image_addon()) {
//		fprintf(stderr, "failed to load image addon!\n");
//		return false;
//	}

//	if (!al_init_font_addon()) {
//		fprintf(stderr, "failed to load font addon!\n");
//		return false;
//	}

//	if (!al_init_native_dialog_addon()) {
//		fprintf(stderr, "failed to load native dialog addon!\n");
//		return false;
//	}

//	al_set_new_display_flags(ALLEGRO_OPENGL);
//	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);

	ALLEGRO_DISPLAY* display = al_create_display(1280, 720);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return false;
	}

	ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
	if (!eventQueue) {
		fprintf(stderr, "failed to create event queue!\n");
		return false;
	};

	al_register_event_source(eventQueue, al_get_display_event_source(display));
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
	al_register_event_source(eventQueue, al_get_mouse_event_source());

	// Dear Imgui
	DearImguiIntegration::Init(display);

	// lua
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	// time management
	double lastTime = al_get_time();
	double dtTarget = 1.0 / 60.0;

	bool stayOpen = true;
	float ctest = 0;
	while (stayOpen) {
		double time = al_get_time();

		ALLEGRO_EVENT event;
		while (al_get_next_event(eventQueue, &event)) {
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				stayOpen = false;
			}
			else if (!DearImguiIntegration::Event(event)) {
				// other events
			}
		}

		DearImguiIntegration::NewFrame();

		ImGui::Begin("Debug");
		ImGui::Text("Hello, world %d", 123);
		if (ImGui::Button("Run test.lua")) {
			if (luaL_dofile(L, "test.lua")) {
				std::cout << lua_tostring(L, -1) << "\n";
			}
		}
		//ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
		ImGui::SliderFloat("float", &ctest, 0.0f, 1.0f);
		ImGui::End();

		al_clear_to_color(al_map_rgba_f(ctest, ctest, ctest, 1.0f));

		DearImguiIntegration::Render();

		al_flip_display();

		double elapsed = al_get_time() - time;

		if (elapsed < dtTarget) {
			al_rest(dtTarget - elapsed);
		}
	}

	lua_close(L);

	DearImguiIntegration::Destroy();

	if (display) { al_destroy_display(display); }
	if (eventQueue) { al_destroy_event_queue(eventQueue); }

	al_uninstall_system();
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
