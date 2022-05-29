#include <iostream>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "imgui_impl_allegro5.h"
#include <allegro5/allegro_primitives.h>
#include "lua/lua.hpp"
#include "TextEditor.h"
#include <fstream>
#include <sstream>
#include "stdcapture.h"
#include "ImFileDialog.h"
#include <objbase.h>
#include "imgui_lua_bindings.h"
#include "additional_bindings.h"
#include <set>
#include "utils.h"

extern "C" int luaopen_lallegro_core(lua_State * L);

thread_local ImGuiContext* MyImGuiTLS;

static double last_loop_start = 0.0;
static bool kill_lua = false;
static ALLEGRO_MUTEX* lua_monitoring_mutex;

static ALLEGRO_DISPLAY* main_display = nullptr;

namespace DearImguiIntegration {
	void Init(ALLEGRO_DISPLAY* display) {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
	//ALLEGRO_DISPLAY* display = al_create_display(280, 420);
	//if (!display) {
	//	fprintf(stderr, "failed to create display!\n");
	//	return (void*)1;
	//}

	al_set_target_backbuffer(main_display);

	ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
	if (!eventQueue) {
		fprintf(stderr, "failed to create event queue!\n");
		return (void*)1;
	};

	al_register_event_source(eventQueue, al_get_display_event_source(main_display));
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
	al_register_event_source(eventQueue, al_get_mouse_event_source());

	// Dear Imgui
	DearImguiIntegration::Init(main_display);

	// time management
	double lastTime = al_get_time();
	double dtTarget = 1.0 / 10.0;

	// taking over stuff
	bool taking_over = false;
	// screenshot doesn't work because it takes the current buffer where nothing was drawn yet
	//ALLEGRO_BITMAP* screenshot = nullptr;

	bool stayOpen = true;
	while (stayOpen) {
		double time = al_get_time();

		al_lock_mutex(lua_monitoring_mutex);
		bool needs_taking_over = (al_get_time() - last_loop_start) > 1.0;
		al_unlock_mutex(lua_monitoring_mutex);

		if (needs_taking_over && !taking_over) {
			// started taking over
			dtTarget = 1.0 / 60.0;
			//if (screenshot) {
			//	al_destroy_bitmap(screenshot);
			//}
			//ALLEGRO_BITMAP* bb = al_get_backbuffer(main_display);
			//screenshot = al_create_bitmap(al_get_bitmap_width(bb), al_get_bitmap_height(bb));
			//al_set_target_bitmap(screenshot);
			//al_draw_bitmap(bb, 0.0f, 0.0f, 0);
			//al_set_target_bitmap(bb);
		}
		else if (!needs_taking_over && taking_over) {
			// stopped taking over
			dtTarget = 1.0 / 10.0;
			//if (screenshot) {
			//	al_destroy_bitmap(screenshot);
			//	screenshot = nullptr;
			//}
		}

		taking_over = needs_taking_over;

		if (taking_over) {
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

			ImGui::Text("Infinite loop detected!");
			if (ImGui::Button("Kill current lua script")) {
				al_lock_mutex(lua_monitoring_mutex);
				kill_lua = true;
				al_unlock_mutex(lua_monitoring_mutex);
			}

			ImGui::End();

			//if (screenshot) {
			//	al_draw_bitmap(screenshot, 0.0f, 0.0f, 0);
			//}
			//else {
				al_clear_to_color(al_map_rgb(0,0,0));
			//}

			DearImguiIntegration::Render();

			al_flip_display();
		}
		else {
			al_flush_event_queue(eventQueue);
		}

		double elapsed = al_get_time() - time;

		if (al_get_thread_should_stop(thread)) {
			stayOpen = false;
		}

		if (elapsed < dtTarget) {
			al_rest(dtTarget - elapsed);
		}
	}

	DearImguiIntegration::Destroy();

	//if (display) { al_destroy_display(display); }
	if (eventQueue) { al_destroy_event_queue(eventQueue); }

	return nullptr;
}

void lua_monitoring_hook(lua_State* L, lua_Debug* ar) {
	al_lock_mutex(lua_monitoring_mutex);
	if (kill_lua) {
		kill_lua = false;
		al_unlock_mutex(lua_monitoring_mutex);
		lua_pushfstring(L, "Lua script killed from debugger");
		lua_error(L); // never returns
		return; // I'm explicit about this anyway
	}
	al_unlock_mutex(lua_monitoring_mutex);
}

static thread_local lua_State* currentluacontext = nullptr;

void ImGuiFriendlyLuaAssert(bool pass, const char* message) {
	if (pass) return;
	if (currentluacontext) {
		std::string str(message);
		size_t message_pos = str.rfind("&& ");
		if (message_pos != std::string::npos) {
			std::string sub = str.substr(message_pos + 3, std::string::npos);
			str = std::string("IMGUI ASSERT : ") + sub;
		}
		lua_pushfstring(currentluacontext, str.c_str());
		lua_error(currentluacontext);
		return;
	}
	assert(pass && message);
}

inline void SafeLuaStart(lua_State* L) {
	currentluacontext = L;
}
bool SafeLuaDefaultError(int retvalue) {
	if (retvalue) {
		std::cout << lua_tostring(currentluacontext, -1) << std::endl;
		lua_pop(currentluacontext, 1);
		return false;
	}
	return true;
}
inline void SafeLuaEnd() {
	ImGui::LuaBindings::CleanEndStack();
	currentluacontext = nullptr;
}

static bool legacyconsole_openstate = true;
int lua_knotbag_legacyconsole(lua_State* L) {
	if (lua_gettop(L) >= 1) {
		legacyconsole_openstate = lua_toboolean(L, 1);
	}
	lua_pushboolean(L, legacyconsole_openstate);
	return 1;
}

void trigger_knotbag_callback(lua_State* L, const char* callback) {
	int pop = 1;
	if (lua_getglobal(L, "knotbag") == LUA_TTABLE) {
		if (lua_getfield(L, -1, callback) == LUA_TFUNCTION) {
			SafeLuaStart(L);
			SafeLuaDefaultError(lua_pcall(L, 0, 0, 0));
			SafeLuaEnd();
		}
		else {
			++pop;
		}
	}
	lua_pop(L, pop);
}

int main()
{
	CoInitialize(nullptr); // ImFileDialog needs this if we don't want the debug output complaining

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

	if (!al_init_image_addon()) {
		fprintf(stderr, "failed to load image addon!\n");
		return false;
	}

//	if (!al_init_font_addon()) {
//		fprintf(stderr, "failed to load font addon!\n");
//		return false;
//	}

//	if (!al_init_native_dialog_addon()) {
//		fprintf(stderr, "failed to load native dialog addon!\n");
//		return false;
//	}

	al_set_new_display_flags(ALLEGRO_RESIZABLE);
//	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);

	main_display = al_create_display(1280, 720);
	if (!main_display) {
		fprintf(stderr, "failed to create display!\n");
		return false;
	}

	ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
	if (!eventQueue) {
		fprintf(stderr, "failed to create event queue!\n");
		return false;
	};

	al_register_event_source(eventQueue, al_get_display_event_source(main_display));
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
	al_register_event_source(eventQueue, al_get_mouse_event_source());

	// Dear Imgui
	DearImguiIntegration::Init(main_display);

	// File Dialog
	ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
		ALLEGRO_BITMAP* bmp = al_create_bitmap(w, h);
		ALLEGRO_LOCKED_REGION* lr = al_lock_bitmap(bmp, (fmt == 0) ? ALLEGRO_PIXEL_FORMAT_ARGB_8888 : ALLEGRO_PIXEL_FORMAT_ABGR_8888, ALLEGRO_LOCK_WRITEONLY);
		memcpy(lr->data, data, w * h * lr->pixel_size); // I mostly ignore lr, will it work?
		al_unlock_bitmap(bmp);
		return (void*)bmp;
	};
	std::vector<ALLEGRO_BITMAP*> bitmaps_to_destroy;
	ifd::FileDialog::Instance().DeleteTexture = [&bitmaps_to_destroy](void* tex) {
		bitmaps_to_destroy.push_back((ALLEGRO_BITMAP*)tex);
	};
	ifd::FileDialog& fileDialog = ifd::FileDialog::Instance();

	lua_State* L;
	// Console
	ImGuiTextBuffer capture;
	ImGuiTextBuffer lastcapture;
	std::capture::CaptureStdout capturer([&capture, &lastcapture, &L](const char* buf, size_t szbuf) {
		// give it to lua if the users wants a custom console
		int pop = 1;
		bool callsuccess = false;
		if (lua_getglobal(L, "knotbag") == LUA_TTABLE) {
			if (lua_getfield(L, -1, "console_callback") == LUA_TFUNCTION) {
				SafeLuaStart(L);
				lua_pushstring(L, buf);
				callsuccess = SafeLuaDefaultError(lua_pcall(L, 1, 0, 0));
				SafeLuaEnd();
			}
			else {
				++pop;
			}
		}
		lua_pop(L, pop);
		if (!callsuccess) {
			capture.append(buf);
		}
		lastcapture.append(buf);
		});

	// lua
	L = luaL_newstate();
	luaL_openlibs(L);
	ImGui::LuaBindings::Load(L);
	luaopen_lallegro_core(L);
	additional_bindings(L);
	lua_newtable(L);
	lua_pushcfunction(L, lua_knotbag_legacyconsole);
	lua_setfield(L, -2, "legacy_console");
	lua_setglobal(L, "knotbag");
	if (fileexists("start.lua")) {
		SafeLuaStart(L);
		SafeLuaDefaultError(luaL_dofile(L, "start.lua"));
		SafeLuaEnd();
	}
	else {
		std::cout << "start.lua doesn't exist, it's probably why you don't see much yet" << std::endl;
	}

	// lua monitoring
	lua_sethook(L, lua_monitoring_hook, LUA_MASKCOUNT, 100);
	lua_monitoring_mutex = al_create_mutex();
	ALLEGRO_THREAD* thread = al_create_thread(lua_monitoring, NULL);
	al_start_thread(thread);

	// Text editors
	std::unordered_map<std::string, std::tuple<TextEditor*, bool, int>> lua_editors;
	std::set<int> lua_editors_ids;
	std::tuple<std::string, TextEditor*, int> editor_saving_as("", nullptr, -1);
	std::string lastFocusedEditor;
	bool editorNeedsFocus = false;
	auto InsertNewLuaEditor = [&lua_editors,&lua_editors_ids, &lastFocusedEditor, &editorNeedsFocus](const std::string& name, bool needsSaveAs) {
		TextEditor* te = new TextEditor;
		te->SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
		int newid = 1;
		while (lua_editors_ids.find(newid++) != lua_editors_ids.end());
		lua_editors_ids.insert(--newid);
		lua_editors[name] = std::make_tuple(te, needsSaveAs, newid);
		lastFocusedEditor = name;
		editorNeedsFocus = true;
		return te;
	};
	auto EditorNewFile = [&lua_editors,&InsertNewLuaEditor]() {
		std::string newfilename = "New lua script";
		if (lua_editors.find(newfilename) != lua_editors.end()) {
			newfilename.append(" ");
			int newnew = 1;
			while (lua_editors.find(newfilename + std::to_string(newnew++)) != lua_editors.end());
			newfilename += std::to_string(--newnew);
		}
		InsertNewLuaEditor(newfilename, true);
	};
	auto EditorOpenFile = [&fileDialog]() {
		fileDialog.Open("MultiPurposeOpen", "Open a file", "Lua script (*.lua){.lua},.*");
	};
	auto EditorSaveFile = [&fileDialog, &editor_saving_as](std::unordered_map<std::string, std::tuple<TextEditor*, bool, int>>::iterator it, bool saveas) {
		if (saveas) {
			fileDialog.Save("SaveLuaScriptAs", "Save the script", "Lua script (*.lua){.lua}");
			std::get<0>(editor_saving_as) = it->first;
			std::get<1>(editor_saving_as) = std::get<0>(it->second);
			std::get<2>(editor_saving_as) = std::get<2>(it->second);
		}
		else {
			TextEditor& editor = *std::get<0>(it->second);
			auto textToSave = editor.GetText();
			std::ofstream t(it->first);
			if (t.good()) {
				t << textToSave;
				editor.MarkSaved();
				return true;
			}
		}
		return false;
	};
	auto EditorRun = [&L](std::unordered_map<std::string, std::tuple<TextEditor*, bool, int>>::iterator& it) {
		std::cout << "Running " << it->first << std::endl;
		SafeLuaStart(L);
		if (luaL_dostring(L, std::get<0>(it->second)->GetText().c_str())) {
			std::string str(lua_tostring(L, -1));
			// replace [string ....]: error by the file name
			size_t message_pos = str.find("\"]:"); // this should do most of cases
			if (message_pos != std::string::npos) {
				std::string sub = str.substr(message_pos + 2, std::string::npos);
				str = it->first + sub;
			}
			std::cout << str << std::endl;
			lua_pop(L, 1);
		}
		SafeLuaEnd();
	};
	auto EditorClose = [&lua_editors_ids, &lua_editors](std::unordered_map<std::string, std::tuple<TextEditor*, bool, int>>::iterator& it) {
		delete std::get<0>(it->second);
		lua_editors_ids.erase(std::get<2>(it->second));
		it = lua_editors.erase(it);
	};

	// time management
	double lastTime = al_get_time();
	double dtTarget = 1.0 / 60.0;

	bool stayOpen = true;
	while (stayOpen) {
		double time = al_get_time();

		al_lock_mutex(lua_monitoring_mutex);
		last_loop_start = time;
		al_unlock_mutex(lua_monitoring_mutex);

		ALLEGRO_EVENT event;
		while (al_get_next_event(eventQueue, &event)) {
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				stayOpen = false;
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(main_display);
			}
			else if (!DearImguiIntegration::Event(event)) {
				// other events
			}
		}

		al_clear_to_color(al_map_rgba_f(0.5f, 0.5f, 0.5f, 1.0f));
		DearImguiIntegration::NewFrame();

		ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

		auto editorit = lua_editors.find(lastFocusedEditor);
		TextEditor* editor = editorit == lua_editors.end() ? nullptr : std::get<0>(editorit->second);

		// Main menu
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl-N")) {
					EditorNewFile();
				}
				if (ImGui::MenuItem("Open", "Ctrl-O")) {
					EditorOpenFile();
				}
				if (ImGui::MenuItem("Save", "Ctrl-S", nullptr, editor)) {
					EditorSaveFile(editorit, std::get<1>(editorit->second));
				}
				if (ImGui::MenuItem("Save as", nullptr, nullptr, editor)) {
					EditorSaveFile(editorit, true);
				}
				if (ImGui::MenuItem("Run", "Ctrl-R", nullptr, editor)) {
					EditorRun(editorit);
				}
				if (ImGui::MenuItem("Quit", "Ctrl-Q", nullptr, editor)) {
					EditorClose(editorit);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, editor && editor->CanUndo()))
					editor->Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, editor && editor->CanRedo()))
					editor->Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor && editor->HasSelection()))
					editor->Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, editor && editor->HasSelection()))
					editor->Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, editor && editor->HasSelection()))
					editor->Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, editor && ImGui::GetClipboardText() != nullptr))
					editor->Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr, editor))
					editor->SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor->GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			//if (ImGui::BeginMenu("View"))
			//{
			//	if (ImGui::MenuItem("Dark palette"))
			//		editor.SetPalette(TextEditor::GetDarkPalette());
			//	if (ImGui::MenuItem("Light palette"))
			//		editor.SetPalette(TextEditor::GetLightPalette());
			//	if (ImGui::MenuItem("Retro blue palette"))
			//		editor.SetPalette(TextEditor::GetRetroBluePalette());
			//	ImGui::EndMenu();
			//}

			//if (ImGui::BeginMenu("Windows")) {
			//	ImGui::MenuItem("Legacy console", nullptr, &legacyconsole_openstate);
			//	ImGui::EndMenu();
			//}

			ImGui::EndMainMenuBar();
		}

		// Handle key shortcuts
		if (ImGui::GetIO().KeyCtrl) {
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N))) {
				EditorNewFile();
			}
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O))) {
				EditorOpenFile();
			}
			if (editor) {
				if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S))) {
					EditorSaveFile(editorit, std::get<1>(editorit->second));
				}
				else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_R))) {
					EditorRun(editorit);
				}
				else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Q))) {
					EditorClose(editorit);
				}
			}
		}

		// function that gets called automatically each frame, the rest is lua's code reponsibility
		trigger_knotbag_callback(L, "framescript");

		bool console_updated = capturer.end();
		if (console_updated) {
			std::cout << lastcapture.c_str();
			lastcapture.clear();
		}
		capturer.start();

		if (legacyconsole_openstate) {
			if (ImGui::Begin("Legacy console", &legacyconsole_openstate)) {
				if (ImGui::Button("Clear"))
					capture.clear();
				ImGui::BeginChild("Console text");
				ImGui::TextUnformatted(capture.begin(), capture.end());
				if (console_updated)
					ImGui::SetScrollHereY(1.0f);
				ImGui::EndChild();
			}
			ImGui::End();
		}

		// lua editors
		for (auto it = lua_editors.begin(); it != lua_editors.end();) {
			bool is_opened = true;
			const std::string& file = it->first;
			const int editor_id = std::get<2>(it->second);
			TextEditor& editor = *std::get<0>(it->second);
			if (editorNeedsFocus && it == editorit) {
				editorNeedsFocus = false;
				ImGui::SetNextWindowFocus();
			}
			ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;
			if (editor.IsModified()) {
				flags |= ImGuiWindowFlags_UnsavedDocument;
			}
			if (ImGui::Begin((file + std::string("###lua_editor n") + std::to_string(editor_id)).c_str(), &is_opened, flags)) {
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
					lastFocusedEditor = file;
				}
				ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
				editor.Render("TextEditor");

				ImVec2 work_pos = ImGui::GetWindowPos();
				ImVec2 work_size = ImGui::GetWindowSize();
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove;
				const float PAD = 20.0f;
				ImVec2 window_pos;
				window_pos.x = work_pos.x + work_size.x - PAD;
				window_pos.y = work_pos.y + work_size.y - PAD;
				ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
				if (ImGui::BeginChild("Cursor position overlay", ImVec2(190.0f, 20.0f), false, window_flags))
				{
					auto cpos = editor.GetCursorPosition();
					ImGui::Text("%6d/%-6d %6d lines", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines());
				}
				ImGui::EndChild();
			}
			if (!is_opened) {
				EditorClose(it);
			}
			else {
				++it;
			}
			ImGui::End();
		}

		// Answer to open file
		if (fileDialog.IsDone("MultiPurposeOpen")) {
			if (fileDialog.HasResult()) {
				std::string ext = fileDialog.GetResult().extension().u8string();
				for (auto& c : ext) { c = std::tolower(c); } //lowercase
				if (ext == ".lua") {
					const std::string res = fileDialog.GetStrLocalResult();

					if (lua_editors.find(res) == lua_editors.end()) {
						std::cout << "Now opening " << res << std::endl;

						std::ifstream t(res);
						if (t.good())
						{
							TextEditor* te = InsertNewLuaEditor(res, false);
							std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
							te->SetText(str);
						}
						else {
							std::cout << "Something went wrong, couldn't open \"" << res << '\"' << std::endl;
						}
					}
					else {
						std::cout << res << " is already opened!" << std::endl;
					}
				}
				else {
					std::cout << "File type \"" << ext << "\" not supported." << std::endl;
				}
			}
			fileDialog.Close();
		}

		// Answer to save file as
		if (fileDialog.IsDone("SaveLuaScriptAs")) {
			if (fileDialog.HasResult() && std::get<1>(editor_saving_as)) {
				std::string res = fileDialog.GetStrLocalResult();
				if (EditorSaveFile(editorit, false)) {
					std::cout << "Saved file \"" << res << '\"' << std::endl;
					auto it = lua_editors.find(std::get<0>(editor_saving_as));
					if (it != lua_editors.end()) {
						lua_editors.erase(it);
					}
					lua_editors[res] = std::make_tuple(std::get<1>(editor_saving_as), false, std::get<2>(editor_saving_as));
				}
				else {
					std::cout << "Something went wrong while saving \"" << res << '\"' << std::endl;
				}
				std::get<0>(editor_saving_as).clear();
				std::get<1>(editor_saving_as) = nullptr;
				std::get<2>(editor_saving_as) = -1;
			}
			fileDialog.Close();
		}

		// End frame
		DearImguiIntegration::Render();
		al_flip_display();

		// Clean frame
		for (auto bmp : bitmaps_to_destroy) {
			al_destroy_bitmap(bmp);
		}
		bitmaps_to_destroy.clear();

		// Wait next frame
		double elapsed = al_get_time() - time;
		if (elapsed < dtTarget) {
			al_rest(dtTarget - elapsed);
		}
	}

	// Notify quit
	trigger_knotbag_callback(L, "quit");

	// Memory leaks happen if we don't do this and the dialog is open while we close everything
	ifd::FileDialog::Instance().Close();

	al_join_thread(thread, nullptr);
	al_destroy_thread(thread);
	al_destroy_mutex(lua_monitoring_mutex);

	lua_close(L);

	DearImguiIntegration::Destroy();

	if (main_display) { al_destroy_display(main_display); }
	if (eventQueue) { al_destroy_event_queue(eventQueue); }

	al_uninstall_system();
	CoUninitialize(); // see CoInitialize at the top of main
}
