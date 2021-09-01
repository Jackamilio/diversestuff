#include "Engine.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "LevelData.h"
#include "Dump.h"
#include "Editorcamera.h"
#include "Exposing.h"
#include <map>

#include "imgui.h"
#include "imgui_impl_allegro5.h"

ALLEGRO_FONT* fetchDefaultFont()
{
	static ALLEGRO_FONT* defaultFont = NULL;
	if (!defaultFont) {
	//	defaultFont = al_
	// _ttf_font("arial.ttf", 14, 0);
	//	if (!defaultFont) {
			defaultFont = al_create_builtin_font();
	//	}
	}
	return defaultFont;
}


class TestCamera : public Engine::Input, public Engine::Update {
public:
	Engine & engine;
	EditorCamera camera;
	//btRigidBody* trackbody;

	bool mleft;

	TestCamera(Engine& e) : engine(e) {
		mleft = false;
		//trackbody = 0;

		engine.inputRoot.AddChild(this);
		engine.updateRoot.AddChild(this);

		camera.distance = 10.0f;
		camera.SetFocusPoint(0, 0, 0.5f);
		camera.SetUp(0, 0, 1);

		engine.graphics.proj = glm::perspective(glm::radians(40.0f), 1280.0f / 720.0f, 0.5f, 50.0f);
	}

	bool Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
			mleft = true;
			return true;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
			mleft = false;
			return true;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && mleft) {
			camera.horAngle += (float)event.mouse.dx * 0.01f;
			camera.verAngle += (float)event.mouse.dy * 0.01f;
			return true;
		}
		return false;
	}

	void Step() {

		/*if (trackbody) {
			btTransform tr = trackbody->getCenterOfMassTransform();
			camera.SetFocusPoint(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
		}*/

		camera.CalcMatrix(engine.graphics.view);

		//ImGui::Begin("Debug camera");
		//ImGui::SliderFloat("Distance", &camera.distance, 0.1f, 50.0f);
		//ImGui::End();
	}
};

class EngineLevel : public Engine::Graphic {
public:
	Engine & engine;
	LevelData lvldt;
	const Model* model;

	//btTriangleMesh* levelMesh;
	//btBvhTriangleMeshShape* shape;
	//btMotionState* motion;
	//btRigidBody* body;

	EngineLevel(Engine& e, const char* level) : engine(e) {
		lvldt.OldLoad(level);
		model = &engine.graphics.models.Get(&lvldt);
		//DrawLevelData(lvldt, engine.graphics.textures, true);

		/*btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));
		levelMesh = ConstructLevelCollision(lvldt);
		shape = new btBvhTriangleMeshShape(levelMesh, true);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, shape);
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);*/

		engine.mainGraphic.AddChildToProgram(this, "test.pgr");
	}

	~EngineLevel() {
		//delete body;
		//delete motion;
		//delete shape;
		//delete levelMesh;
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4(1.0));
		model->Draw();
	}
};

class LevelEditor : public Engine::Input, public Engine::Update, public Engine::Graphic {
public:
	Engine& engine;
	EngineLevel& lvl;
	LevelData& lvldt;
	bool showGui;
	bool lastShowGui;
	int lastTileset;
	char tsfile[64] = "";
	union {
		int tilesetmemory[6]{ 0 };
		struct {
			int tsox, tsoy, tspx, tspy, tsw, tsh;
		};
	};
	struct SelectedTile {
		int set = -1, x = 0, y = 0;
	};
	SelectedTile selectedtile{};
	int TSIDtoDel;

	LevelEditor(EngineLevel& lvl) :
		engine(lvl.engine),
		lvl(lvl),
		lvldt(lvl.lvldt),
		showGui(false),
		lastShowGui(false),
		lastTileset(-1),
		TSIDtoDel(-1)
	{
		engine.inputRoot.AddChild(this);
		engine.updateRoot.AddChild(this);
	}

	bool Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_KEY_UP && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			showGui = !showGui;
			return true;
		}
		return false;
	}

	void LoadTilesetInGui(int tileset) {
		const LevelData::TilesetData* tsd = lvldt.GetTilesetData(tileset);
		if (tsd) {
			strcpy_s(tsfile, 64, tsd->file.c_str());
			memcpy(tilesetmemory, tsd->values, sizeof(tilesetmemory));
		}
	}

	void Step() {
		//when showing the gui is toggled
		if (showGui != lastShowGui) {
			lastShowGui = showGui;

			if (showGui) {
				// show the debug level, and not the correct one anymore
				engine.mainGraphic.RemoveChildFromProgram(&lvl, "test.pgr");
				engine.mainGraphic.AddChild(this);
			}
			else {
				// show the correct level, and not the debug one anymore
				engine.mainGraphic.RemoveChild(this);

				// tell him to reload
				engine.graphics.models.RemoveValue(&lvldt);
				lvl.model = &engine.graphics.models.Get(&lvldt);

				engine.mainGraphic.AddChildToProgram(&lvl, "test.pgr");
			}
		}

		if (showGui) {
			if (ImGui::Begin("Level editor", &showGui)) {
				if (ImGui::CollapsingHeader("Tilesets")) {
					if (ImGui::BeginTabBar("Tilesets")) {
						int wantTSIDtoDel = -1;
						int curTileset = 0;
						const LevelData::TilesetData* tsd = lvldt.GetTilesetData(curTileset);
						while (tsd) {
							char tabtitle[64];
							sprintf_s(tabtitle, 64, "Tileset %i", curTileset);
							
							bool open = true;
							if (ImGui::BeginTabItem(tabtitle, &open)) {
								if (curTileset != lastTileset) {
									lastTileset = curTileset;
									LoadTilesetInGui(curTileset);
								}

								{
									bool change = false;
									// file dialog to choose the picture
									if (ImGui::Button("...")) {
										ALLEGRO_FILECHOOSER* dialog = al_create_native_file_dialog(
											tsd->file.c_str(),
											"Choose a picture for the tileset",
											"*.jpg;*.bmp;*.png",
											ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_PICTURES);
										if (al_show_native_file_dialog(engine.display, dialog)) {
											const char* answer = al_get_native_file_dialog_path(dialog, 0);
											std::string result = makefilelocal(answer);
											if (result.compare(tsd->file) != 0) {
												change = true;
												strcpy_s(tsfile, 64, result.c_str());
											}
										}
										al_destroy_native_file_dialog(dialog);
									}
									ImGui::SameLine();
									change = change || ImGui::InputText("Picture file", tsfile, 64, ImGuiInputTextFlags_CharsNoBlank);

									// tilsedata values tweaking
									if (ImGui::BeginTable("Tilset values", 2)) {
										ImGui::TableNextColumn();
										change = change || ImGui::InputInt("Tile offset X", &tsox);
										ImGui::TableNextColumn();
										change = change || ImGui::InputInt("Tile offset Y", &tsoy);

										ImGui::TableNextColumn();
										change = change || ImGui::InputInt("Tile padding X", &tspx);
										ImGui::TableNextColumn();
										change = change || ImGui::InputInt("Tile padding Y", &tspy);

										ImGui::TableNextColumn();
										change = change || ImGui::InputInt("Tile width", &tsw);
										ImGui::TableNextColumn();
										change = change || ImGui::InputInt("Tile height", &tsh);

										ImGui::EndTable();
									}

									// update the level and its undo/redo if a change was detected
									if (change) {
										lvldt.UpdateTilesetData(curTileset, tsfile, tsox, tsoy, tspx, tspy, tsw, tsh);
									}
								}


								// don't show the tiles if they're too small, that makes too many vertices for ImGui
								if (tsd->tw >= 8 && tsd->th >= 8) {
									// select if tiles are shown above their original texture, or as individual without context
									static bool showtilesonly = false;
									ImGui::Checkbox("Show tiles only", &showtilesonly);

									const Texture& tex = engine.graphics.textures.Get(tsd->file);

									ImVec2 imgorigin = ImGui::GetCursorPos();
									const float texw = tex.GetWidth();
									const float texh = tex.GetHeight();
									if (!showtilesonly) {
										ImGui::Image(tex.GetAlValue(), ImVec2(texw, texh));
									}
									ImVec2 endpos = ImGui::GetCursorPos();

									// double loop to show tiles as buttons, with frame padding calculation
									const int p = 1;
									const int p2 = p * 2;
									ImVec2 buttonsize(tsw - p2, tsh - p2);
									ImVec2 texpos(tsox, tsoy);
									ImVec2 cursorpos(0, 0);
									int ty = 0;
									while (texpos.y + tspy + tsh <= texh) {
										texpos.x = tsox;
										cursorpos.x = 0;
										int tx = 0;
										while (texpos.x + tspx + tsw <= texw) {
											if (!showtilesonly) {
												cursorpos = texpos;
											}
											ImGui::SetCursorPos(ImVec2(imgorigin.x + cursorpos.x, imgorigin.y + cursorpos.y));

											// the tile button, give it a unique ID for ImGui
											ImGui::PushID(tx | ty << 12);
											if (ImGui::ImageButton(tex.GetAlValue(), buttonsize,
												ImVec2((texpos.x + p) / texw, (texpos.y + p) / texh),
												ImVec2((texpos.x + tsw - p) / texw, (texpos.y + tsh - p) / texh),
												p,
												ImVec4(0, 0, 0, 0),
												// green tint if selected, white otherwise
												(selectedtile.set == curTileset && selectedtile.x == tx && selectedtile.y == ty) ? ImVec4(0.2f, 1, 0.2f, 1) : ImVec4(1, 1, 1, 1)))
											{
												// Set this one as selected if the user clicks on it
												selectedtile.set = curTileset;
												selectedtile.x = tx;
												selectedtile.y = ty;
											}
											ImGui::PopID();

											texpos.x += tspx + tsw;
											cursorpos.x += tsw + 2;
											++tx;
										}
										texpos.y += tspy + tsh;
										cursorpos.y += tsh + 2;
										++ty;
									}

									if (!showtilesonly) {
										ImGui::SetCursorPos(endpos);
									}
								}

								ImGui::EndTabItem();
 							}

							// if this tab was closed, it means the user wants to delete it
							if (!open) {
								wantTSIDtoDel = curTileset;
							}
							tsd = lvldt.GetTilesetData(++curTileset);
						}

						// button to add a new tileset
						if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
							lvldt.AddTilesetData();
						}

						ImGui::EndTabBar();

						// popup to confirm deleting a tileset
						if (wantTSIDtoDel >= 0) {
							TSIDtoDel = wantTSIDtoDel;
							ImGui::OpenPopup("Delete?");
						}

						if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							ImGui::Text("Are you sure you want to remove this tileset?\nResults are currently undefined as removing is still WIP\n\n");
							ImGui::Separator();

							if (ImGui::Button("Yes", ImVec2(120, 0)))
							{
								lvldt.RemoveTilesetData(TSIDtoDel);
								lastTileset = -1;
								TSIDtoDel = -1;

								ImGui::CloseCurrentPopup();
							}
							ImGui::SetItemDefaultFocus();
							ImGui::SameLine();
							if (ImGui::Button("No", ImVec2(120, 0))) {
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}
					}
				}
				//ImGui::Text("Selected tileset : %i", curTileset);
			}
			ImGui::End();
		}

		ImGui::ShowDemoWindow();
	}

	void Draw() {
		DrawLevelData(lvldt, engine.graphics.textures, true);
	}
};

class ExposingTestMember {
public:
	int _otherInt;
	short _otherShort;
	bool _otherBool;

	ExposingTestMember()
		: _otherInt(0)
		, _otherShort(0)
		, _otherBool(true)
	{}

	IM_AN_EXPOSER
};

#define EXPOSE_TYPE ExposingTestMember
EXPOSE_START
EXPOSE(_otherInt)
EXPOSE(_otherShort)
EXPOSE(_otherBool)
EXPOSE_END
#undef EXPOSE_TYPE

class ExposingTest {
public:
	bool _bool;
	char _char;
	unsigned char _uchar;
	short _short;
	unsigned short _ushort;
	int _int;
	unsigned int _uint;
	float _float;
	double _double;
	std::string _string;
	ExposingTestMember _member;
	std::vector<int> _vector;
	std::vector<ExposingTestMember> _vectorMember;
	std::map<std::string, int> _mapMember;

	struct NotExposed {
		int val;
	};

	NotExposed _notExposed{};
	
	ExposingTest()
		: _bool(false)
		, _char(0)
		, _uchar(0)
		, _short(0)
		, _ushort(0)
		, _int(0)
		, _uint(0)
		, _float(0.0f)
		, _double(0.0)
		, _string("string")
	{
		_vector.push_back(0);
		_vector.push_back(1);

		ExposingTestMember member;
		_vectorMember.push_back(member);
		member._otherInt = 1;
		_vectorMember.push_back(member);
		member._otherBool = true;
		_vectorMember.push_back(member);

		_mapMember["one"] = 1;
		_mapMember["two"] = 2;
		_mapMember["three"] = 3;
	}

	IM_AN_EXPOSER
};

#define EXPOSE_TYPE ExposingTest
EXPOSE_START
EXPOSE(_mapMember)
EXPOSE(_bool)
EXPOSE(_char)
EXPOSE(_uchar)
EXPOSE(_short)
EXPOSE(_ushort)
EXPOSE(_member)
EXPOSE(_notExposed)
EXPOSE(_int)
EXPOSE(_uint)
EXPOSE(_float)
EXPOSE(_double)
EXPOSE(_string)
EXPOSE(_vector)
EXPOSE(_vectorMember)
EXPOSE_END
#undef EXPOSE_TYPE

class FPSCounter : public Engine::Graphic {
public:
	Engine & engine;

	double samplingDuration;
	double lastTime;
	int nbSamples;
	int lastFps;

	FPSCounter(Engine& e) : engine(e) {
		engine.overlayGraphic.AddChild(this);

		samplingDuration = 0.5;
		lastTime = engine.time;
		nbSamples = 0;
		lastFps = (int)(1.0 / engine.dtTarget);
	}

	void Draw() {

		++nbSamples;
		if (engine.time - lastTime > samplingDuration) {
			lastFps = (int)((double)nbSamples / samplingDuration);
			nbSamples = 0;
			lastTime = engine.time;
		}

		al_draw_textf(fetchDefaultFont(), al_map_rgb(255, 255, 255), 10, 10, 0, "FPS : %i", lastFps);
	}
};

int main(int nbarg, char ** args) {
	Engine engine;

	engine.graphics.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	engine.graphics.pointLights[0][0] = glm::vec4(5.0f, 0.0f, 1.0f, 3.0f);
	engine.graphics.pointLights[0][1] = glm::vec4(0.7f, 0.0f, 0.0f, 0.0f);
	engine.graphics.pointLights[1][0] = glm::vec4(0.0f, 1.0f, 0.75f, 1.5f);
	engine.graphics.pointLights[1][1] = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);

	if (engine.Init()) {
		al_init_primitives_addon();
		al_init_ttf_addon();

		TestCamera camera(engine);
		EngineLevel lvl(engine,"niveau.lvl");
		LevelEditor editor(lvl);
		FPSCounter fc(engine);

		while (engine.OneLoop()) {}
	}
}
