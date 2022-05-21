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

extern "C" int luaopen_lallegro_core(lua_State * L);

thread_local ImGuiContext* MyImGuiTLS;

static double last_loop_start = 0.0;
static bool kill_lua = false;
static ALLEGRO_MUTEX* lua_monitoring_mutex;

typedef std::pair<std::string, bool> Script;
static std::vector<Script> lua_scripts[2];
static int current_lua_scripts = 0;

static ALLEGRO_DISPLAY* main_display = nullptr;

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

	// Console
	ImGuiTextBuffer capture;
	std::capture::CaptureStdout capturer([&capture](const char* buf, size_t szbuf) {
		capture.append(buf);
		//capture += std::string(buf, szbuf);
		});

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

	// Text editors
	std::unordered_map<std::string, std::pair<TextEditor*, bool>> lua_editors;
	std::pair<std::string, TextEditor*> editor_saving_as("", nullptr);

	// lua
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);
	ImGui::LuaBindings::Load(L);
	luaopen_lallegro_core(L);
	additional_bindings(L);

	lua_sethook(L, lua_monitoring_hook, LUA_MASKCOUNT, 100);

	// lua monitoring thread
	lua_monitoring_mutex = al_create_mutex();
	ALLEGRO_THREAD* thread = al_create_thread(lua_monitoring, NULL);
	al_start_thread(thread);

	//al_rest(2.0);

	// gui stuff
	bool win_demo = false;
	bool win_console = true;
	bool win_scripts = true;

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

		DearImguiIntegration::NewFrame();

		ifd::FileDialog& fileDialog = ifd::FileDialog::Instance();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New")) {
					std::string newfilename = "New lua script ";
					if (lua_editors.find(newfilename) != lua_editors.end()) {
						int newnew = 1;
						while (lua_editors.find(newfilename + std::to_string(newnew++)) != lua_editors.end());
						newfilename += std::to_string(--newnew);
					}
					TextEditor* te = new TextEditor;
					te->SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
					lua_editors[newfilename] = std::make_pair(te, true);
				}
				if (ImGui::MenuItem("Open")) {
					fileDialog.Open("MultiPurposeOpen", "Open a file", "Lua script (*.lua){.lua},.*");
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows")) {
				ImGui::MenuItem("Console", nullptr, &win_console);
				ImGui::MenuItem("Scripts", nullptr, &win_scripts);
				ImGui::Separator();
				ImGui::MenuItem("Demo", nullptr, &win_demo);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

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
							TextEditor* te = new TextEditor;
							te->SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
							std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
							te->SetText(str);
							lua_editors[res] = std::make_pair(te, false);
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

		if (win_demo) {
			ImGui::ShowDemoWindow(&win_demo);
		}

		if (win_console) {
			if (ImGui::Begin("Console", &win_console)) {
				if (ImGui::Button("Clear"))
					capture.clear();
				bool updated = capturer.flush();
				ImGui::BeginChild("Console text");
				ImGui::TextUnformatted(capture.begin(), capture.end());
				if (updated)
					ImGui::SetScrollHereY(1.0f);
				ImGui::EndChild();
			}
			ImGui::End();
		}

		if (win_scripts) {
			if (ImGui::Begin("Scripts", &win_scripts)) {
				std::vector<Script>& cur_scripts = lua_scripts[current_lua_scripts];
				if (ImGui::Button("Kill selected")) {
					for (auto it = cur_scripts.begin(); it != cur_scripts.end();) {
						if (it->second) {
							it = cur_scripts.erase(it);
						}
						else {
							++it;
						}
					}
				}
				int i = 0;
				for (auto& script : cur_scripts) {
					ImGui::PushID(i++);
					if (ImGui::Selectable(script.first.c_str(), script.second))
					{
						script.second ^= 1;
					}
					ImGui::PopID();
				}
			}
			ImGui::End();
		}

		auto EditorSaveFile = [&fileDialog, &editor_saving_as](TextEditor& editor, const std::string& file, bool saveas) {
			if (saveas) {
				fileDialog.Save("SaveLuaScriptAs", "Save the script", "Lua script (*.lua){.lua}");
				editor_saving_as.first = file;
				editor_saving_as.second = &editor;
			}
			else {
				auto textToSave = editor.GetText();
				std::ofstream t(file);
				if (t.good()) {
					t << textToSave;
					return true;
				}
			}
			return false;
		};

		for (auto it = lua_editors.begin(); it != lua_editors.end();) {
			bool is_opened = true;
			const std::string& file = it->first;
			bool wants_quit = false;
			if (ImGui::Begin((file + std::string("###") + std::to_string((int)it->second.first)).c_str(), &is_opened, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings)) {
				if (!is_opened) {
					wants_quit = true;
				}
				else {
					TextEditor& editor = *it->second.first;
					ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu("File"))
						{
							if (ImGui::MenuItem("Save", "Ctrl-S")) {
								EditorSaveFile(editor, file, it->second.second);
							}
							if (ImGui::MenuItem("Save as")) {
								EditorSaveFile(editor, file, true);
							}
							if (ImGui::MenuItem("Run", "Ctrl-R")) {
								if (it->second.second) {
									std::cout << "Please save the file before running." << std::endl;
								}
								else {
									lua_scripts[current_lua_scripts].push_back(Script(file, false));
								}
							}
							if (ImGui::MenuItem("Quit", "Ctrl-Q")) {
								wants_quit = true;
							}
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

					auto cpos = editor.GetCursorPosition();
					ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
						editor.IsOverwrite() ? "Ovr" : "Ins",
						editor.CanUndo() ? "*" : " ",
						editor.GetLanguageDefinition().mName.c_str());

					if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl) {
						if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S))) {
							EditorSaveFile(editor, file, it->second.second);
						}
						else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_R))) {
							lua_scripts[current_lua_scripts].push_back(Script(file,false));
						}
						else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Q))) {
							wants_quit = true;
						}
					}
					editor.Render("TextEditor");
				}
			}
			if (wants_quit) {
				delete it->second.first;
				it = lua_editors.erase(it);
			}
			else {
				++it;
			}
			ImGui::End();
		}

		if (fileDialog.IsDone("SaveLuaScriptAs")) {
			if (fileDialog.HasResult() && editor_saving_as.second) {
				std::string res = fileDialog.GetResult().u8string();
				if (EditorSaveFile(*editor_saving_as.second, res, false)) {
					std::cout << "Saved file \"" << res << '\"' << std::endl;
					auto it = lua_editors.find(editor_saving_as.first);
					if (it != lua_editors.end()) {
						lua_editors.erase(it);
					}
					lua_editors[res] = std::make_pair(editor_saving_as.second, false);
				}
				else {
					std::cout << "Something went wrong while saving \"" << res << '\"' << std::endl;
				}
				editor_saving_as.first.clear();
				editor_saving_as.second = nullptr;
			}
			fileDialog.Close();
		}

		al_clear_to_color(al_map_rgba_f(0.5f, 0.5f, 0.5f, 1.0f));
		
		currentluacontext = L;
		const int next_scripts = (current_lua_scripts + 1) % 2;
		for (auto script : lua_scripts[current_lua_scripts]) {
			if (luaL_dofile(L, script.first.c_str())) {
				std::cout << lua_tostring(L, -1) << "\n";
			}
			else if (lua_isboolean(L, -1)) {
				if (lua_toboolean(L, -1)) {
					lua_scripts[next_scripts].push_back(script);
				}
			}
			lua_settop(L, 0);
			ImGui::LuaBindings::CleanEndStack();
		}
		lua_scripts[current_lua_scripts].clear();
		current_lua_scripts = next_scripts;
		currentluacontext = nullptr;

		DearImguiIntegration::Render();

		al_flip_display();

		for (auto bmp : bitmaps_to_destroy) {
			al_destroy_bitmap(bmp);
		}
		bitmaps_to_destroy.clear();

		double elapsed = al_get_time() - time;

		if (elapsed < dtTarget) {
			al_rest(dtTarget - elapsed);
		}
	}

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
