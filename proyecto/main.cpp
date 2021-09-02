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


class TestGraphicTarget : public Engine::DoubleGraphic {
public:
	Engine& engine;
	Engine::GraphicTarget target;

	TestGraphicTarget(Engine& e) :
		engine(e),
		target(256,256)
	{
		engine.graphicTargets.AddChild(&target, true);
		target.clearColor.b = 1.0f;
		target.clearColor.a = 1.0f;

		engine.overlayGraphic.AddChild(this);
		target.AddChild(this->GetSecondGraphic());
	}

	void Draw() {
		al_draw_bitmap(target.bitmap, 10, 50, 0);
	}

	void SecondDraw() {
		al_draw_textf(fetchDefaultFont(), al_map_rgb(255, 255, 255), 10, 10, 0, "hey hiy hoy haaaar");
	}
};

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

class LevelEditor : public Engine::Input, public Engine::Update, public Engine::DoubleGraphic {
public:
	Engine& engine;
	EngineLevel& lvl;
	LevelData& lvldt;
	bool showGui;
	bool lastShowGui;
	int lastTileset;
	int curBrickdata;
	int lastBrickdata;
	LevelData::TilesetData guiTsd;
	std::vector<LevelData::Vertex> guiBdVertices;
	LevelData::BrickData::TriangleList guiBdTriangles;
	struct SelectedTile {
		int set = -1, x = 0, y = 0;
	};
	SelectedTile selectedtile{};
	int TSIDtoDel;
	int BDIDtoDel;

	Engine::GraphicTarget brickDataPreview;
	EditorCamera camera;

	LevelEditor(EngineLevel& lvl) :
		engine(lvl.engine),
		lvl(lvl),
		lvldt(lvl.lvldt),
		showGui(false),
		lastShowGui(false),
		lastTileset(-1),
		curBrickdata(-1),
		lastBrickdata(-1),
		TSIDtoDel(-1),
		BDIDtoDel(-1),
		brickDataPreview(265,256,true)
	{
		engine.inputRoot.AddChild(this);
		engine.updateRoot.AddChild(this);

		camera.SetAngles(1.0f, 0.5f);
		camera.SetDistance(3.0f);
		camera.SetUp(0, 0, 1);
		camera.SetFocusPoint(0.5f, 0.5f, 0.5f);
	}

	bool Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_KEY_UP && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			showGui = !showGui;
			return true;
		}
		return false;
	}

	void LoadBrickdataInGui(int brickdata)
	{
		const LevelData::BrickData* bd = lvldt.GetBrickData(brickdata);
		if (bd) {

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
				engine.graphicTargets.AddChild(&brickDataPreview, true);
				brickDataPreview.AddChild(GetSecondGraphic());
			}
			else {
				// show the correct level, and not the debug one anymore
				engine.mainGraphic.RemoveChild(this);
				engine.graphicTargets.RemoveChild(&brickDataPreview);

				// tell him to reload
				engine.graphics.models.RemoveValue(&lvldt);
				lvl.model = &engine.graphics.models.Get(&lvldt);

				engine.mainGraphic.AddChildToProgram(&lvl, "test.pgr");
			}
		}

		if (showGui) {
			if (ImGui::Begin("Level editor", &showGui)) {
				int wantTSIDtoDel = -1;
				int wantBDIDtoDel = -1;
				if (ImGui::CollapsingHeader("Tilesets")) {
					if (ImGui::BeginTabBar("Tilesets")) {
						int curTileset = 0;
						const LevelData::TilesetData* tsd = lvldt.GetTilesetData(curTileset);
						while (tsd) {
							char tabtitle[64];
							sprintf_s(tabtitle, 64, "Tileset %i", curTileset);
							
							bool open = true;
							if (ImGui::BeginTabItem(tabtitle, &open)) {
								if (curTileset != lastTileset) {
									lastTileset = curTileset;
									//LoadTilesetInGui(curTileset);
									guiTsd = *tsd;
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
												guiTsd.file = result;
											}
										}
										al_destroy_native_file_dialog(dialog);
									}
									ImGui::SameLine();
									char tsfile[64] = "";
									strcpy_s(tsfile, 64, guiTsd.file.c_str());
									change = change || ImGui::InputText("Picture file", tsfile, 64, ImGuiInputTextFlags_CharsNoBlank);
									guiTsd.file = tsfile;

									// tilsedata values tweaking
									if (ImGui::BeginTable("Tilset values", 2)) {
										ImGui::TableNextColumn();
										change = ImGui::InputInt("Tile offset X", &guiTsd.ox) || change;
										ImGui::TableNextColumn();
										change = ImGui::InputInt("Tile offset Y", &guiTsd.oy) || change;

										ImGui::TableNextColumn();
										change = ImGui::InputInt("Tile padding X", &guiTsd.px) || change;
										ImGui::TableNextColumn();
										change = ImGui::InputInt("Tile padding Y", &guiTsd.py) || change;

										ImGui::TableNextColumn();
										change = ImGui::InputInt("Tile width", &guiTsd.tw) || change;
										ImGui::TableNextColumn();
										change = ImGui::InputInt("Tile height", &guiTsd.th) || change;

										ImGui::EndTable();
									}

									// update the level and its undo/redo if a change was detected
									if (change) {
										lvldt.UpdateTilesetData(curTileset, guiTsd.file.c_str(), guiTsd.ox, guiTsd.oy, guiTsd.px, guiTsd.py, guiTsd.tw, guiTsd.th);
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
									ImVec2 buttonsize(guiTsd.tw - p2, guiTsd.th - p2);
									ImVec2 texpos(guiTsd.ox, guiTsd.oy);
									ImVec2 cursorpos(0, 0);
									int ty = 0;
									while (texpos.y + guiTsd.py + guiTsd.th <= texh) {
										texpos.x = guiTsd.ox;
										cursorpos.x = 0;
										int tx = 0;
										while (texpos.x + guiTsd.px + guiTsd.tw <= texw) {
											if (!showtilesonly) {
												cursorpos = texpos;
											}
											ImGui::SetCursorPos(ImVec2(imgorigin.x + cursorpos.x, imgorigin.y + cursorpos.y));

											// the tile button, give it a unique ID for ImGui
											ImGui::PushID(tx | ty << 12);
											if (ImGui::ImageButton(tex.GetAlValue(), buttonsize,
												ImVec2((texpos.x + p) / texw, (texpos.y + p) / texh),
												ImVec2((texpos.x + guiTsd.tw - p) / texw, (texpos.y + guiTsd.th - p) / texh),
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

											texpos.x += guiTsd.px + guiTsd.tw;
											cursorpos.x += guiTsd.tw + 2;
											++tx;
										}
										texpos.y += guiTsd.py + guiTsd.th;
										cursorpos.y += guiTsd.th + 2;
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
					}
				}
				if (ImGui::CollapsingHeader("Bricks")) {
					if (ImGui::BeginTabBar("Brick data sheets")) {
						int curbd = 0;
						LevelData::BrickData* bd = lvldt.GetBrickData(curbd);
						while (bd) {
							char tabtitle[64];
							sprintf_s(tabtitle, 64, "Brick data %i", curbd);

							static int selectedtriangle = -1;
							static int selectedvertex = -1;
							bool open = true;
							if (ImGui::BeginTabItem(tabtitle, &open)) {
								curBrickdata = curbd;
								if (curbd != lastBrickdata) {
									lastBrickdata = curbd;
									guiBdTriangles = bd->GetTriangleList();
									guiBdVertices.clear();
									int vi = 0;
									const LevelData::Vertex* v = bd->GetVertex(vi);
									while (v) {
										guiBdVertices.push_back(*v);
										v = bd->GetVertex(++vi);
									}
									selectedtriangle = -1;
									selectedvertex = -1;
								}

								if (selectedvertex >= guiBdVertices.size()) { selectedvertex = -1; }

								if (ImGui::BeginChild("Fine vertex control", ImVec2(220, 300))) {
									static LevelData::Vertex dummy;
									const bool disabled = (selectedvertex < 0);
									LevelData::Vertex* v = disabled ? &dummy : &guiBdVertices[selectedvertex];
									ImGui::BeginDisabled(disabled);
									ImGui::Text(disabled ? "No vertex selected" : "Vertex %i selected", selectedvertex);
									if (!disabled) {
										ImGui::SameLine();
										if (ImGui::Button("Remove")) {
											bd->RemoveVertex(selectedvertex);
											guiBdVertices.erase(guiBdVertices.begin() + selectedvertex);
											selectedvertex = -1;
										}
									}
									const int vsw = 35, vsh = 270;
									const char* format = "%.2f";
									bool changed = ImGui::VSliderFloat("##x", ImVec2(vsw, vsh), &v->x, 0.0f, 1.0f, format); ImGui::SameLine();
									changed = ImGui::VSliderFloat("##y", ImVec2(vsw, vsh), &v->y, 0.0f, 1.0f, format) || changed; ImGui::SameLine();
									changed = ImGui::VSliderFloat(" ##z", ImVec2(vsw, vsh), &v->z, 0.0f, 1.0f, format) || changed; ImGui::SameLine();
									changed = ImGui::VSliderFloat("##u", ImVec2(vsw, vsh), &v->u, 0.0f, 1.0f, format) || changed; ImGui::SameLine();
									changed = ImGui::VSliderFloat("##v", ImVec2(vsw, vsh), &v->v, 0.0f, 1.0f, format) || changed;
									if (changed) { bd->UpdateVertex(selectedvertex, v->x, v->y, v->z, v->u, v->v); }
									ImGui::EndDisabled();
								}
								ImGui::EndChild();

								ImGui::SameLine();
								ImGui::Image(brickDataPreview.bitmap, ImVec2(256, 256));

								if (ImGui::BeginChild("Vertices", ImVec2(320, 200))) {
									for (int i = 0; i < guiBdVertices.size(); ++i) {
										LevelData::Vertex& v = guiBdVertices[i];
										ImGui::PushID(i);
										char buf[32];
										sprintf_s(buf, 32, "%i:", i);
										if (ImGui::Selectable(buf, selectedvertex == i, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0, 20))) {
											selectedvertex = i;
										}
										ImGui::SameLine();
										ImGui::PushItemWidth(140.0f);
										bool changed = ImGui::InputFloat3("xyz", v.values);
										ImGui::PopItemWidth();
										ImGui::SameLine();
										ImGui::PushItemWidth(90.0f);
										changed = ImGui::InputFloat2("uv", &v.u) || changed;
										ImGui::PopItemWidth();
										ImGui::PopID();
										if (changed) { bd->UpdateVertex(i, v.x, v.y, v.z, v.u, v.v); }
									}
									if (ImGui::Button("Add a vertex")) {
										bd->AddVertex();
										guiBdVertices.push_back(LevelData::Vertex());
										ImGui::Text(" "); //dummy for scroll
										ImGui::SetScrollHereY(0.0f);
									}
								}
								ImGui::EndChild();

								ImGui::SameLine();

								if (ImGui::BeginChild("Triangles section", ImVec2(130, 200))) {

									if (selectedtriangle * 3 > guiBdTriangles.size()) { selectedtriangle = -1; }

									bool change = false;
									const bool disabled = (selectedtriangle < 0);
									ImGui::PushItemWidth(25);
									ImGui::BeginDisabled(disabled);
									ImGui::Text("  "); ImGui::SameLine();
									if (ImGui::Button("-")) {
										const int tripos = selectedtriangle * 3;
										guiBdTriangles.erase(guiBdTriangles.begin() + tripos, guiBdTriangles.begin() + tripos + 3);
										change = true;
									}
									ImGui::SameLine();
									bool refreshscroll = false;
									if (ImGui::Button("^") && selectedtriangle > 0) {
										refreshscroll = true;

										swapmemorychunks(&guiBdTriangles[selectedtriangle * 3], &guiBdTriangles[(selectedtriangle - 1) * 3], sizeof(int[3]));
										--selectedtriangle;
										change = true;
									}
									ImGui::SameLine();
									if (ImGui::Button("v") && selectedtriangle + 1 < guiBdTriangles.size() / 3) {
										refreshscroll = true;

										swapmemorychunks(&guiBdTriangles[selectedtriangle * 3], &guiBdTriangles[(selectedtriangle + 1) * 3], sizeof(int[3]));
										++selectedtriangle;
										change = true;
									}
									ImGui::EndDisabled();
									ImGui::PopItemWidth();

									if (ImGui::BeginChild("Triangles")) {
										ImGui::PushItemWidth(25);
										for (int i = 0; i < guiBdTriangles.size(); ++i) {
											ImGui::PushID(i);
											if (i % 3 == 0) {
												const int triangleid = i / 3;
												if (ImGui::Selectable("##triangleselect", selectedtriangle == triangleid, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0, 20))) {
													selectedtriangle = triangleid;
												}
												ImGui::SameLine();
												if (refreshscroll && selectedtriangle == triangleid) {
													ImGui::SetScrollHereY();
												}
											}
											change = ImGui::InputInt("", &guiBdTriangles[i], 0) || change;
											ImGui::PopID();
											if (i % 3 != 2) { ImGui::SameLine(); }
										}
										ImGui::PopItemWidth();
									}
									if (ImGui::Button("Add a triangle")) {
										ImGui::Text(" "); //dummy for scroll
										ImGui::SetScrollHereY(0.0f);
										guiBdTriangles.push_back(0);
										guiBdTriangles.push_back(0);
										guiBdTriangles.push_back(0);
										change = true;
									}
									ImGui::EndChild();

									if (change) {
										bd->UpdateTriangleList(guiBdTriangles);
									}
								}
								ImGui::EndChild();

								ImGui::EndTabItem();
							}
							
							// if this tab was closed, it means the user wants to delete it
							if (!open) {
								wantBDIDtoDel = curbd;
							}
							bd = lvldt.GetBrickData(++curbd);
						}

						// button to add a new brick data
						if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
							lvldt.AddBrickData();
						}

						ImGui::EndTabItem();
					}
				}

				// popup to confirm deleting a tileset or a brick data
				if (wantTSIDtoDel >= 0) {
					TSIDtoDel = wantTSIDtoDel;
					ImGui::OpenPopup("Delete?");
				}
				if (wantBDIDtoDel >= 0) {
					BDIDtoDel = wantBDIDtoDel;
					ImGui::OpenPopup("Delete?");
				}

				if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					const bool handlingTS = (TSIDtoDel >= 0);
					const bool handlingBD = (BDIDtoDel >= 0);
					// never too cautious!
					if (handlingTS && handlingBD) {
						ImGui::Text("How to did you manage wanting to delete a tileset and a brick data at the same time?\nYou clicked two close button at the same frame?\nARE YOU A NINJA?\n\n");
					}
					else {
						if (handlingTS) {
							ImGui::Text("Are you sure you want to remove this tileset?\nResults are currently undefined as removing is still WIP\n\n");
						}
						else if (handlingBD) {
							ImGui::Text("Are you sure you want to remove this brick data?\nResults are currently undefined as removing is still WIP\n\n");
						}
						ImGui::Separator();

						if (ImGui::Button("Yes", ImVec2(120, 0)))
						{
							if (handlingTS) {
								lvldt.RemoveTilesetData(TSIDtoDel);
								lastTileset = -1;
								TSIDtoDel = -1;
							}
							else if (handlingBD) {
								lvldt.RemoveBrickData(BDIDtoDel);
								lastBrickdata = -1;
								BDIDtoDel = -1;
							}

							ImGui::CloseCurrentPopup();
						}
						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();
						if (ImGui::Button("No", ImVec2(120, 0))) {
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndPopup();
				}
			}

			ImGui::End();
		}

		ImGui::ShowDemoWindow();
	}

	void Draw() {
		DrawLevelData(lvldt, engine.graphics.textures, true);
	}

	void SecondDraw() {
		const LevelData::BrickData* brick = lvldt.GetBrickData(curBrickdata);

		if (brick) {
			glMatrixMode(GL_PROJECTION);
			glm::mat4 projMat = glm::perspective(glm::radians(40.0f), 1.0f, 0.1f, 150.0f);
			glLoadMatrixf(glm::value_ptr(projMat));
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glm::mat4 mat;
			camera.CalcMatrix(mat);
			glMultMatrixf(glm::value_ptr(mat));

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_LIGHTING);
			glColor3f(0.5, 0.5, 0.5);
			DrawGlWireCube(0.0f, 1.0f);

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glColor3ub(255, 255, 255);
			DrawBrick(brick);
		}
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
		TestGraphicTarget tgt(engine);

		while (engine.OneLoop()) {}
	}
}
