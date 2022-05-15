#include <iostream>
#include <allegro5/allegro5.h>
#include "imgui_impl_allegro5.h"
#include <allegro5/allegro_primitives.h>
#include "lua/lua.hpp"
#include "TextEditor.h"
#include <fstream>
#include <sstream>
#include "stdcapture.h"
#include "imgui_lua_bindings.h"

extern "C" int luaopen_lallegro_core(lua_State * L);

thread_local ImGuiContext* MyImGuiTLS;

static double last_loop_start = 0.0;
static bool kill_lua = false;
static ALLEGRO_MUTEX* mutex;

static std::vector<std::string> lua_scripts[2];
static int current_lua_scripts = 0;

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

static void* lua_monitoring(ALLEGRO_THREAD* thread, void* arg) {
	ALLEGRO_DISPLAY* display = al_create_display(280, 420);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return (void*)1;
	}

	ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
	if (!eventQueue) {
		fprintf(stderr, "failed to create event queue!\n");
		return (void*)1;
	};

	al_register_event_source(eventQueue, al_get_display_event_source(display));
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
	al_register_event_source(eventQueue, al_get_mouse_event_source());

	// Dear Imgui
	DearImguiIntegration::Init(display);

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
		if (al_get_thread_should_stop(thread)) {
			stayOpen = false;
		}

		DearImguiIntegration::NewFrame();

		ImGui::Begin("Debug");

		al_lock_mutex(mutex);
		ImGui::Text("Last time loop started %f", last_loop_start);
		if (ImGui::Button("Kill current lua script")) {
			kill_lua = true;
		}
		al_unlock_mutex(mutex);

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

	DearImguiIntegration::Destroy();

	if (display) { al_destroy_display(display); }
	if (eventQueue) { al_destroy_event_queue(eventQueue); }

	return nullptr;
}

void lua_monitoring_hook(lua_State* L, lua_Debug* ar) {
	al_lock_mutex(mutex);
	if (kill_lua) {
		kill_lua = false;
		al_unlock_mutex(mutex);
		lua_pushfstring(L, "Lua script killed from debugger");
		lua_error(L); // never returns
		return; // I'm explicit about this anyway
	}
	al_unlock_mutex(mutex);
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

	// Console
	std::string capture;
	std::capture::CaptureStdout capturer([&capture](const char* buf, size_t szbuf) {
		capture += std::string(buf, szbuf);
		});

	// Text editor
	TextEditor editor;
	editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

	static const char* fileToEdit = "test.lua";
	//	static const char* fileToEdit = "test.cpp";
	{
		std::ifstream t(fileToEdit);
		if (t.good())
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str);
		}
	}

	// lua
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);
	ImGui::LuaBindings::Load(L);
	luaopen_lallegro_core(L);
	lua_sethook(L, lua_monitoring_hook, LUA_MASKCOUNT, 100);

	// lua monitoring thread
	mutex = al_create_mutex();
	ALLEGRO_THREAD* thread = al_create_thread(lua_monitoring, NULL);
	al_start_thread(thread);

	// time management
	double lastTime = al_get_time();
	double dtTarget = 1.0 / 60.0;

	bool stayOpen = true;
	while (stayOpen) {
		double time = al_get_time();

		al_lock_mutex(mutex);
		last_loop_start = time;
		al_unlock_mutex(mutex);

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

		ImGui::Begin("Console");
		if (ImGui::Button("Clear"))
			capture.clear();
		bool updated = capturer.flush();
		ImGui::BeginChild("Console text");
		ImGui::Text(capture.c_str());
		if (updated)
			ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();
		ImGui::End();

		ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

		auto DoSave = [&editor]() {
			auto textToSave = editor.GetText();
			std::ofstream t(fileToEdit);
			if (t.good()) { t << textToSave; }
		};
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save", "Ctrl-S")) {
					DoSave();
				}
				if (ImGui::MenuItem("Quit", "Alt-F4"))
					break;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor.IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor.SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
					editor.Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
					editor.Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
					editor.Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
					editor.Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
					editor.Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor.Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					editor.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor.SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor.SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::Button("Run test.lua")) {
			lua_scripts[current_lua_scripts].push_back("test.lua");
		}

		auto cpos = editor.GetCursorPosition();
		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), fileToEdit);

		ImGuiIO& io = ImGui::GetIO();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S))) {
			DoSave();
		}
		editor.Render("TextEditor");
		ImGui::End();

		al_clear_to_color(al_map_rgba_f(0.5f, 0.5f, 0.5f, 1.0f));

		const int next_scripts = (current_lua_scripts + 1) % 2;
		for (auto script : lua_scripts[current_lua_scripts]) {
			if (luaL_dofile(L, script.c_str())) {
				std::cout << lua_tostring(L, -1) << "\n";
			}
			else if (lua_isboolean(L, -1)) {
				if (lua_toboolean(L, -1)) {
					lua_scripts[next_scripts].push_back(script);
				}
			}
			ImGui::LuaBindings::CleanEndStack();
		}
		lua_scripts[current_lua_scripts].clear();
		current_lua_scripts = next_scripts;

		DearImguiIntegration::Render();

		al_flip_display();

		double elapsed = al_get_time() - time;

		if (elapsed < dtTarget) {
			al_rest(dtTarget - elapsed);
		}
	}

	al_join_thread(thread, nullptr);
	al_destroy_thread(thread);
	al_destroy_mutex(mutex);

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
