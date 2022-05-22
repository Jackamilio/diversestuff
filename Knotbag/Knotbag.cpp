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

const char* luastartupscript =
"\
function fileexists(name)\n\
	local f = io.open(name, \"r\")\n\
	if f~= nil then\n\
		io.close(f)\n\
		return true\n\
	else\n\
		return false\n\
	end\n\
end\n\
\n\
if fileexists(\"start.lua\") then\n\
		dofile(\"start.lua\")\n\
end\
";

const char* luaframescript =
"\
if knotbag and type(knotbag.framescript) == \"function\" then\n\
	knotbag.framescript() \n\
end\
";

bool SafeLuaDoString(lua_State* L, const char* str) {
	bool ret = true;
	currentluacontext = L;
	if (luaL_dostring(L, str)) {
		std::cout << lua_tostring(L, -1) << std::endl;
		ret = false;
	}
	ImGui::LuaBindings::CleanEndStack();
	currentluacontext = nullptr;
	return ret;
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
	ifd::FileDialog& fileDialog = ifd::FileDialog::Instance();

	// lua
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);
	ImGui::LuaBindings::Load(L);
	luaopen_lallegro_core(L);
	additional_bindings(L);
	if (luaL_dostring(L, luastartupscript)) {
		std::cout << "Startup script error : " << lua_tostring(L, -1) << std::endl;
	}

	lua_sethook(L, lua_monitoring_hook, LUA_MASKCOUNT, 100);

	// lua monitoring thread
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
		std::string newfilename = "New lua script ";
		if (lua_editors.find(newfilename) != lua_editors.end()) {
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
			auto textToSave = std::get<0>(it->second)->GetText();
			std::ofstream t(it->first);
			if (t.good()) {
				t << textToSave;
				return true;
			}
		}
		return false;
	};
	auto EditorRun = [&L](std::unordered_map<std::string, std::tuple<TextEditor*, bool, int>>::iterator& it) {
		std::cout << "Running " << it->first << std::endl;
		SafeLuaDoString(L, std::get<0>(it->second)->GetText().c_str());
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

		DearImguiIntegration::NewFrame();

		ImGui::DockSpaceOverViewport();

		//if (win_console) {
		//	if (ImGui::Begin("Console", &win_console)) {
			if (ImGui::Begin("Console")) {
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
		//}

		auto editorit = lua_editors.find(lastFocusedEditor);
		TextEditor* editor = editorit == lua_editors.end() ? nullptr : std::get<0>(editorit->second);

		// lua editors
		for (auto it = lua_editors.begin(); it != lua_editors.end();) {
			bool is_opened = true;
			const std::string& file = it->first;
			const int editor_id = std::get<2>(it->second);
			if (editorNeedsFocus && it == editorit) {
				editorNeedsFocus = false;
				ImGui::SetNextWindowFocus();
			}
			if (ImGui::Begin((file + std::string("###lua_editor n") + std::to_string(editor_id)).c_str(), &is_opened, ImGuiWindowFlags_HorizontalScrollbar)) {
				TextEditor& editor = *std::get<0>(it->second);
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
					lastFocusedEditor = file;
				}
				const bool needsSaveAs = std::get<1>(it->second);
				ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

				auto cpos = editor.GetCursorPosition();
				ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
					editor.IsOverwrite() ? "Ovr" : "Ins",
					editor.CanUndo() ? "*" : " ",
					editor.GetLanguageDefinition().mName.c_str());

				editor.Render("TextEditor");
			}
			if (!is_opened) {
				EditorClose(it);
			}
			else {
				++it;
			}
			ImGui::End();
		}

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

		al_clear_to_color(al_map_rgba_f(0.5f, 0.5f, 0.5f, 1.0f));
		
		// function that gets called automatically each frame, the rest is lua's code reponsibility
		SafeLuaDoString(L, luaframescript);

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
