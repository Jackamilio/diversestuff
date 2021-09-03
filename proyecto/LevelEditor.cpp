#include "LevelEditor.h"
#include "Dump.h"
#include "imgui.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_native_dialog.h>

LevelEditor::LevelEditor(EngineLevel& lvl) :
	engine(lvl.engine),
	lvl(lvl),
	lvldt(lvl.lvldt),
	showGui(false),
	lastShowGui(false),
	tilemode(false),
	lastTileset(-1),
	curBrickdata(-1),
	lastBrickdata(-1),
	TSIDtoDel(-1),
	BDIDtoDel(-1),
	brickHeapPreview(128, 128, true),
	brickDataPreview(265, 256, true),
	levelCamera(2.4f),
	gridUp(0,0,1)
{
	engine.inputRoot.AddChild(this, true); //add on top
	engine.updateRoot.AddChild(this);

	bdPreviewCamera.SetAngles(1.0f, 0.5f);
	bdPreviewCamera.SetDistance(3.0f);
	bdPreviewCamera.SetUp(0, 0, 1);
	bdPreviewCamera.SetFocusPoint(0.5f, 0.5f, 0.5f);
	brickDataPreview.AddChild(GetSecondGraphic());

	brickHeapPreview.AddChild(GetThirdGraphic());

	levelCamera.SetAngles(1.0f, 0.5f);
	levelCamera.SetUp(0, 0, 1);
	levelCamera.SetFocusPoint(0.5f, 0.5f, 0.5f);

	lerper.target = &levelCamera.focuspoint;
}

bool LevelEditor::Event(ALLEGRO_EVENT& event) {
	if (!showGui) {
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			showGui = true;
			tilemode = false; // always start without tilemode?
		}
		return showGui;
	}
	else {
		if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (event.keyboard.keycode == ALLEGRO_KEY_F1) {
				showGui = false;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_T) {
				tilemode = !tilemode;
			}
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			if (al_key_down(&keyboardState, ALLEGRO_KEY_LCTRL)) {
				const int eventdz = event.mouse.dz;
				if (eventdz != 0) {
					gridCenter.x += eventdz * (int)gridUp.x;
					gridCenter.y += eventdz * (int)gridUp.y;
					gridCenter.z += eventdz * (int)gridUp.z;
				}
			}
			else  {
				levelCamera.Zoom(event);
				levelCamera.Drag(event, 1);
			}
		}
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 3 && al_key_down(&keyboardState, ALLEGRO_KEY_LSHIFT)) {
			if (gridUp.x == 1.0f) {
				gridUp = glm::vec3(-1.0f, 0.0f, 0.0f);
			}
			else if (gridUp.x == -1.0f) {
				gridUp = glm::vec3(0.0f, 1.0f, 0.0f);
			}
			else if (gridUp.y == 1.0f) {
				gridUp = glm::vec3(0.0f, -1.0f, 0.0f);
			}
			else if (gridUp.y == -1.0f) {
				gridUp = glm::vec3(0.0f, 0.0f, 1.0f);
			}
			else if (gridUp.z == 1.0f) {
				gridUp = glm::vec3(0.0f, 0.0f, -1.0f);
			}
			else if (gridUp.z == -1.0f) {
				gridUp = glm::vec3(1.0f, 0.0f, 0.0f);
			}
			return true;
		}
		
		/*static double lastclick = 0.0;

		doubleclick = false;
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 3) {
			double newclick = al_get_time();
			doubleclick = (newclick - lastclick) <= 0.4;
			lastclick = newclick;
		}*/

		const bool buttondown = event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
		const bool rightheld = mouseState.buttons & 2 || (buttondown && event.mouse.button == 2);
		const bool middleclick = buttondown && event.mouse.button == 3;
		if (rightheld || middleclick) {
			HandlePick( rightheld, middleclick);
		}
		return true; // block any event regardless
	}
}

void LevelEditor::HandlePick(bool rightheld, bool middleclick)
{
	glm::mat4 cam;
	levelCamera.CalcMatrix(cam);
	int w = al_get_bitmap_width(engine.defaultGraphicTarget.bitmap);
	int h = al_get_bitmap_height(engine.defaultGraphicTarget.bitmap);
	glm::vec4 vp(0, 0, w, h);

	glm::vec3 mouse((float)mouseState.x, (float)(h - mouseState.y), 1.0f);
	glm::vec3 worldMouse = glm::unProject(mouse, cam, engine.graphics.proj, vp);

	glm::vec3 plane_center = gridCenter.asVec3();
	if (gridUp.x + gridUp.y + gridUp.z < 0.0f) {
		plane_center -= gridUp;
	}
	glm::vec3 pick;
	glm::vec3 rayDir = glm::normalize(worldMouse - levelCamera.GetPosition());

	LevelData::RayCastResult res;
	bool levelcollision = lvldt.RayCast(levelCamera.GetPosition(), rayDir, 70.0f, &res);

	bool gridcollision = linePlaneIntersection(plane_center, gridUp, levelCamera.GetPosition(), rayDir, true, &pick);
	LevelData::Coordinate c(
		gridUp.x != 0.0f ? gridCenter.x : (int)glm::floor(pick.x),
		gridUp.y != 0.0f ? gridCenter.y : (int)glm::floor(pick.y),
		gridUp.z != 0.0f ? gridCenter.z : (int)glm::floor(pick.z));

	if (middleclick) {
		// middle button : camera center selection
		if (al_key_down(&keyboardState, ALLEGRO_KEY_LCTRL)) {
			if (levelcollision) {
				/*A = res.intersection;

				const LevelData::Brick& brick = guidata->leveldata->GetBrick(res.coordinate)[res.brickIndex];
				brick.TransformTriangle(res.coordinate.asVec3(), res.triangleIndex, T1, T2, T3);

				glm::vec3 tn = glm::normalize(glm::cross(T2 - T1, T3 - T1));
				B = A + tn;*/

				gridCenter = res.coordinate;
				GlideToNewCenter();
			}
		}
		else if (gridcollision) {
			gridCenter = c;
			GlideToNewCenter();
		}
	}
	else if (rightheld) {
		// right button : modifying stuff
		if (tilemode) {
			if (levelcollision) {
				if (al_key_down(&keyboardState, ALLEGRO_KEY_LSHIFT)) {
					// copy tile
					const LevelData::Brick& brick = lvldt.GetBrick(res.coordinate)[res.brickIndex];
					selectedtile.set = lvldt.FindTilesetData(brick.tilesetdata);
					selectedtile.x = brick.tilex;
					selectedtile.y = brick.tiley;
				}
				else {
					// draw tile or remove tile
					lvldt.UpdateBrickTile(res.coordinate, res.brickIndex, al_key_down(&keyboardState, ALLEGRO_KEY_LCTRL) ? -1 : selectedtile.set, selectedtile.x, selectedtile.y);
				}
			}
		}
		else if (gridcollision && c != lastClic) {
			if (al_key_down(&keyboardState, ALLEGRO_KEY_LSHIFT)) {
				// copy brick
				brickheap = lvldt.GetBrick(c);
			}
			else if (al_key_down(&keyboardState, ALLEGRO_KEY_LCTRL)) {
				// remove brick
				lvldt.ClearBrick(c);
			}
			else {
				// place brick
				lvldt.StackBrick(c, brickheap);
			}
		}
	}

	if (gridcollision) {
		lastClic = c;
	}
}

void LevelEditor::GlideToNewCenter() {
	lerper.Set(levelCamera.focuspoint, gridCenter.asVec3() + glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 16);
	lerper.Begin();
}


void LevelEditor::Draw() {
	// change view locally
	glPushMatrix();
	glm::mat4 mat;
	levelCamera.CalcMatrix(mat);
	glLoadMatrixf(glm::value_ptr(mat));

	// draw the level
	glColor3ub(255, 255, 255);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	DrawLevelData(lvldt, engine.graphics.textures, true);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);

	// grid
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glColor3ub(255, 255, 255);
	glPushMatrix();
	glTranslatef((float)gridCenter.x, (float)gridCenter.y, (float)gridCenter.z);

	if (gridUp.x == 1.0f) {
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	}
	else if (gridUp.x == -1.0f) {
		glTranslatef(1.0f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	}
	else if (gridUp.y == 1.0f) {
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	}
	else if (gridUp.y == -1.0f) {
		glTranslatef(0.0f, 1.0f, 0.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	}
	else if (gridUp.z == 1.0f) {
		// default: nothing
	}
	else if (gridUp.z == -1.0f) {
		glTranslatef(0.0f, 0.0f, 1.0f);
	}

	glBegin(GL_LINES);
	{
		const int range = 7;
		for (int i = -range; i <= range + 1; ++i) {
			glVertex3i(i, -range, 0); glVertex3i(i, range + 1, 0);
			glVertex3i(-range, i, 0); glVertex3i(range + 1, i, 0);
		}
	}
	glEnd();
	glPopMatrix();

	// highlighted position
	//glColor3ub(255, 255, 0);
	//glPushMatrix();
	//glTranslatef((float)gridCenter.x + 0.5f, (float)gridCenter.y + 0.5f, (float)gridCenter.z + 0.5f);
	//DrawGlWireCube(-0.6f, 0.6f);
	//glPopMatrix();

	glPopMatrix();
}

void LevelEditor::SecondDraw() {
	const LevelData::BrickData* brick = lvldt.GetBrickData(curBrickdata);

	if (brick) {
		glMatrixMode(GL_PROJECTION);
		glm::mat4 projMat = glm::perspective(glm::radians(40.0f), 1.0f, 0.1f, 150.0f);
		glLoadMatrixf(glm::value_ptr(projMat));
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glm::mat4 mat;
		bdPreviewCamera.CalcMatrix(mat);
		glMultMatrixf(glm::value_ptr(mat));

		glDisable(GL_TEXTURE_2D);

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

void LevelEditor::ThirdDraw() {
	if (!tilemode) {
		glMatrixMode(GL_PROJECTION);
		glm::mat4 projMat = glm::perspective(glm::radians(40.0f), 1.0f, 0.1f, 150.0f);
		glLoadMatrixf(glm::value_ptr(projMat));
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glm::mat4 mat;
		EditorCamera copycam = levelCamera;
		copycam.focuspoint = glm::vec3(0.5f, 0.5f, 0.5f);
		copycam.distance = 2.0f;
		copycam.CalcMatrix(mat);
		copycam.CalcMatrix(mat);
		glMultMatrixf(glm::value_ptr(mat));

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glColor3ub(255, 255, 255);
		DrawBrickHeap(brickheap, engine.graphics.textures, true);
	}
}

void LevelEditor::Step() {
	//when showing the gui is toggled
	if (showGui != lastShowGui) {
		lastShowGui = showGui;

		if (showGui) {
			// show the debug level, and not the correct one anymore
			engine.mainGraphic.RemoveChildFromProgram(&lvl, "test.pgr");
			engine.mainGraphic.AddChild(this);
			engine.graphicTargets.AddChild(&brickDataPreview, true);
			engine.graphicTargets.AddChild(&brickHeapPreview, true);
		}
		else {
			// show the correct level, and not the debug one anymore
			engine.mainGraphic.RemoveChild(this);
			engine.graphicTargets.RemoveChild(&brickDataPreview);
			engine.graphicTargets.RemoveChild(&brickHeapPreview);

			// tell him to reload
			engine.graphics.models.RemoveValue(&lvldt);
			lvl.model = &engine.graphics.models.Get(&lvldt);

			engine.mainGraphic.AddChildToProgram(&lvl, "test.pgr");
		}
	}

	if (showGui) {
		if (lerper.t < 1.0f) {
			lerper.Step();
		}

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

							if (ImGui::Button("Add to heap")) {
								LevelData::Brick b(lvldt);
								b.brickdata = bd;
								brickheap.push_back(b);
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
							if (ImGui::BeginChild("Preview", ImVec2(300, 300))) {
								ImGui::Image(brickDataPreview.bitmap, ImVec2(256, 256));
								ImGui::SameLine();
								ImGui::VSliderFloat("##cam control ver", ImVec2(20, 256), &bdPreviewCamera.verAngle, -glm::half_pi<float>() * 0.999f, glm::half_pi<float>() * 0.999f, "");
								ImGui::PushItemWidth(256);
								ImGui::DragFloat("##cam control hor", &bdPreviewCamera.horAngle, 0.005f, -FLT_MAX, +FLT_MAX, "", 0);
								ImGui::PopItemWidth();
							}
							ImGui::EndChild();

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

		// Tile and break heap preview
		if (ImGui::Begin("Tile/Brick preview"))
		{
			const ImVec2 previewSize(128, 128);
			if (tilemode) {
				if (ImGui::Button("Tile mode")) {
					tilemode = !tilemode;
				}
				const LevelData::TilesetData* tsd = lvldt.GetTilesetData(selectedtile.set);
				if (tsd) {
					const Texture& tex = engine.graphics.textures.Get(tsd->file);
					const float texw = tex.GetWidth();
					const float texh = tex.GetHeight();
					const ImVec2 tpos(tsd->ox + selectedtile.x * (tsd->px + tsd->tw), tsd->oy + selectedtile.y * (tsd->py + tsd->th));
					ImGui::Image(tex.GetAlValue(), previewSize,
						ImVec2(tpos.x / texw, tpos.y / texh),
						ImVec2((tpos.x + tsd->tw) / texw, (tpos.y + tsd->th) / texh));
				}
				else {
					const Texture& tex = engine.graphics.textures.Get("wrongtexture");
					ImGui::Image(tex.GetAlValue(), previewSize);
				}
			}
			else {
				if (ImGui::Button("Brick mode")) {
					tilemode = !tilemode;
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear")) {
					brickheap.clear();
				}
				ImGui::Image(brickHeapPreview.bitmap, previewSize);
			}
		}
		ImGui::End();
	}

	ImGui::ShowDemoWindow();
}
