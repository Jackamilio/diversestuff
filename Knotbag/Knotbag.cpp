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
#include "imgui_lua_bindings.h"
#include "ImFileDialog.h"
#include <objbase.h>

extern "C" int luaopen_lallegro_core(lua_State * L);

thread_local ImGuiContext* MyImGuiTLS;

static double last_loop_start = 0.0;
static bool kill_lua = false;
static ALLEGRO_MUTEX* mutex;

typedef std::pair<std::string, bool> Script;
static std::vector<Script> lua_scripts[2];
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
	lua_sethook(L, lua_monitoring_hook, LUA_MASKCOUNT, 100);

	// lua monitoring thread
	mutex = al_create_mutex();
	ALLEGRO_THREAD* thread = al_create_thread(lua_monitoring, NULL);
	al_start_thread(thread);

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
				ImGui::MenuItem("Demo", nullptr, &win_demo);
				ImGui::MenuItem("Scripts", nullptr, &win_scripts);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (fileDialog.IsDone("MultiPurposeOpen")) {
			if (fileDialog.HasResult()) {
				std::string ext = fileDialog.GetResult().extension().u8string();
				for (auto& c : ext) { c = std::tolower(c); } //lowercase
				if (ext == ".lua") {
					const std::string& res = fileDialog.GetResult().u8string();
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
				ImGui::Text(capture.c_str());
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
			ImGui::PushID(it->second.first); // The editor is the ID, not the title
			if (ImGui::Begin((file + std::string("###")).c_str(), &is_opened, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar)) {
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
			ImGui::PopID();
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
			ImGui::LuaBindings::CleanEndStack();
		}
		lua_scripts[current_lua_scripts].clear();
		current_lua_scripts = next_scripts;

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
	al_destroy_mutex(mutex);

	lua_close(L);

	DearImguiIntegration::Destroy();

	if (display) { al_destroy_display(display); }
	if (eventQueue) { al_destroy_event_queue(eventQueue); }

	al_uninstall_system();
	CoUninitialize(); // see CoInitialize at the top of main
}
