#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Bar.H>
#include <Fl/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Box.H>
#include <Fl/Fl_Scroll.H>
#include <Fl/Fl_File_Chooser.H>
#include <Fl/Fl_Table.H>
#include <FL/Fl_Gl_Window.H>
#include <Fl/Fl_Native_File_Chooser.H>
#include <Fl/Fl_Select_Browser.h>
#include <Fl/gl.h>
#include <FL/glu.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>
#include <list>

#include "gui.h"
#include "gui_setup.h"
#include "dump.h"
#include "mathutils.h"
#include "editorcamera.h"
#include "interpolator.h"

// Code found here : http://seriss.com/people/erco/fltk/#Fl_Resize_Browser
// Manage arranging thumbnails in a viewer
//     Assumes thumbnails are all the same size.
//
class MyThumbnailViewer : public Fl_Scroll {
	int xsep;
	int ysep;

	// Rearrange the children based on new xywh
	void RearrangeChildren(int X, int Y, int W, int H) {
		int nchildren = children() - 2;   // -2: ignore Fl_Scroll's two child scrollbars
		if (nchildren < 1) return;
		int sw = (scrollbar.visible()) ? scrollbar.w() : 0;
		// Figure out how many thumbnails we can arrange horizontally
		int thumb_w = (child(0)->w() + xsep);     // overall width of thumbnail, including separation
		int thumb_h = (child(0)->h() + ysep);     // overall height of thumbnail, including separation
		int nw = (W - sw - xsep) / thumb_w;  // max # thumbnails we can arrage horizontally before wrapping
		int remain = (W - sw - xsep) % thumb_w;  // remainder of division
		if (nw < 2) nw = 2;                     // minimum number of thumbs
		int left = X + xsep + (remain / 2);         // left edge of thumb arrangement
		int top = Y + ysep;                      // top edge of thumb arrangement
												 // Now walk the children, moving them around
		int tx = left;
		int ty = top;
		for (int i = 0; i<nchildren; i++) {
			Fl_Widget *w = child(i);
			w->position(tx, ty);
			if ((i % nw) == (nw - 1)) {         // wrap?
				ty += thumb_h;                      // advance to next row
				tx = left;                          // reset to left
			}
			else {                      // no wrap?
				tx += thumb_w;              // just advance to next right position
			}
		}
	}
public:
	void resize(int X, int Y, int W, int H) {
		RearrangeChildren(X, Y, W, H);     // move children around to fit new size
		Fl_Scroll::resize(X, Y, W, H);     // let Fl_Scroll handle the new arrangement
	}

	MyThumbnailViewer(int X, int Y, int W, int H, const char *L = 0) : Fl_Scroll(X, Y, W, H, L) {
		xsep = 5;
		ysep = 5;
	}
	// Set the x/y separation between thumbnails
	void separation(int X, int Y) {
		xsep = X;
		ysep = Y;
		autoarrange();
	}
	// Force auto-arrangement of children
	void autoarrange() {
		RearrangeChildren(x(), y(), w(), h());
		// init_sizes();    // XXX: doesn't help
		Fl_Scroll::redraw();
	}
	void draw() {
		// XXX: this trick shouldn't be needed, but without it
		//      children initially start in really strange positions
		static int first = 1;
		if (first) { autoarrange(); first = 0; }
		Fl_Scroll::draw();
	}
};

class TileButton : public Fl_Group
{
public:
	TileButton(int index, GuiSharedData* rd)
		: Fl_Group(10, 10, 100, 100, 0)
		, index(index)
		, guidata(rd)
		, image(0)
		, group(0)
		, edit(0)
		, remove(0)
	{
		box(FL_UP_BOX);

		begin();

		edit = new Fl_Button(x() + 5, y() + 75, 40, 20, "@%redo");
		remove = new Fl_Button(x() + 55, y() + 75, 40, 20, "@1+");

		edit->callback(EditTilesetCallback, (void*)this);
		remove->callback(RemoveTileButtonCallback, (void*)this);

		end();
	}

	~TileButton() {
	}

	int index;
	GuiSharedData* guidata;
	Fl_Image* image;
protected:
	void draw() {
		if (guidata->cur_ts_id == index) {
			color(FL_RED);
		}
		else {
			color(FL_GRAY);
		}
		Fl_Group::draw();
		if (image)
			image->draw(x() + 5, y() + 5, 90, 70);
	}

	int handle(int event) {
		switch (event) {
		case FL_PUSH:
			if (Fl::event_button() == FL_LEFT_MOUSE) {

				// ignore for ourself if a child can take the push
				for (int i = children(); i--;) {
					if (Fl::event_inside(child(i))) {
						return Fl_Group::handle(event);
					}
				}

				guidata->cur_ts_id = index;
				guidata->scroller->redraw();
				ReloadTilechooserCallback(this, (void*)guidata);
				return 1;
			}
			// intentional fall through
		default:
			return Fl_Group::handle(event);
		}
	}
private:
	Fl_Group* group;
	Fl_Button* edit;
	Fl_Button* remove;
};

class TilesetPreview : public Fl_Box {
public:
	TilesetPreview(int X, int Y, TilesetManagerData* tmdata) : Fl_Box(X, Y, 2000, 2000), tmdt(tmdata) {}
	/*void image(Fl_Image* img) {
		Fl_Widget::image(img);
		if (img) {
			size(img->w(), img->h());
		}
	}*/
protected:
	void draw() {
		Fl_Image* img = Fl_Widget::image();
		if (img) {
			img->draw(x(), y(), img->w(), img->h());

			if (tmdt) {
				int ox = (int)tmdt->tileset_ox->value();
				int oy = (int)tmdt->tileset_oy->value();
				int px = (int)tmdt->tileset_px->value();
				int py = (int)tmdt->tileset_py->value();
				int tw = (int)tmdt->tileset_tw->value();
				int th = (int)tmdt->tileset_th->value();
				if (th > 2 && tw > 2
					&& ox >= 0 && oy >= 0
					&& px >= 0 && py >= 0) {

					for (int Y = oy; Y<img->h(); Y += py + th)
						for (int X = ox; X<img->w(); X += px + tw)
							fl_draw_box(FL_BORDER_FRAME, x() + X, y() + Y, tw, th, FL_WHITE);
				}
			}
		}
	}
private:
	TilesetManagerData* tmdt;
};

class TileChooser : public Fl_Box {
public:
	TileChooser(GuiSharedData* gd) : Fl_Box(0,0,2000,2000), tsdt(0), image(0), guidata(gd)
	{
		box(FL_FLAT_BOX);
		color(FL_GRAY);
	}

	void draw() {
		Fl_Box::draw();
		fl_color(FL_BLACK);
		if (!tsdt) {
			fl_draw("No tileset available.", 10, 20);
		}
		else if (!image) {
			fl_draw("Tileset has no image file, or couldn't load it.", 10, 20);
		}
		else if (tsdt->th < 2 || tsdt->tw < 2
			|| tsdt->ox < 0 || tsdt->oy < 0
			|| tsdt->px < 0 || tsdt->py < 0) {
			fl_draw("Tileset properties incorrect.", 10, 20);
		}
		else {
			int j = 0;
			for (int Y = tsdt->oy; Y < image->h(); Y += tsdt->py + tsdt->th) {
				int i = 0;
				for (int X = tsdt->ox; X < image->w(); X += tsdt->px + tsdt->tw) {

					int xx = 5 + (3 + tsdt->tw) * i;
					int yy = 5 + (3 + tsdt->th) * j;

					image->draw(xx, yy, tsdt->tw, tsdt->th, X, Y);

					if (i == guidata->cur_tile_x && j == guidata->cur_tile_y) {
						fl_draw_box(FL_BORDER_FRAME, xx - 1, yy - 1, tsdt->tw + 2, tsdt->th + 2, FL_YELLOW);
						fl_draw_box(FL_BORDER_FRAME, xx - 2, yy - 2, tsdt->tw + 4, tsdt->th + 4, FL_YELLOW);
						fl_draw_box(FL_BORDER_FRAME, xx - 3, yy - 3, tsdt->tw + 6, tsdt->th + 6, FL_YELLOW);
					}

					++i;
				}
				++j;
			}
		}
	}

	void draw_label() {};

	int handle(int event) {
		switch (event) {
		case FL_PUSH:
			if (Fl::event_button() == FL_LEFT_MOUSE) {

				if (tsdt) {
					int mx = Fl::event_x() - 5;
					int my = Fl::event_y() - 5;
					int x = mx / (tsdt->tw + 3);
					int y = my / (tsdt->tw + 3);

					if (x >= 0 && y >= 0
						&& mx <= (int)((3 + tsdt->tw) * (image->w() - tsdt->ox) / (tsdt->tw + tsdt->px))
						&& my <= (int)((3 + tsdt->th) * (image->h() - tsdt->oy) / (tsdt->th + tsdt->py))) {
						guidata->cur_tile_x = x;
						guidata->cur_tile_y = y;
						redraw();

						if (Fl::event_clicks() == 1) {
							window()->hide();
						}
					}
				}

				return 1;
			}
			// intentional fall through
		default:
			return Fl_Box::handle(event);
		}
	}

	void Reload() {
		tsdt = guidata->leveldata->GetTilesetData(guidata->cur_ts_id);
		image = tsdt ? Fl_Shared_Image::get(tsdt->file.c_str()) : 0;
		redraw();
	}

private:
	const LevelData::TilesetData* tsdt;
	Fl_Image* image;
	GuiSharedData* guidata;
};

class GlPreview : public Fl_Gl_Window {
public:
	GlPreview(int X, int Y, int W, int H, GuiSharedData* gd, const char*L = 0)
		: Fl_Gl_Window(X, Y, W, H, L)
		, guidata(gd)
		, distPower(1.2f)
	{
		camera.SetDistance(pow(glm::e<float>(), distPower));
	}

	int handle(int e) {
		const float pixToAngle = 0.01f;
		switch (e) {
		case FL_PUSH:
			if (Fl::event_button() == FL_RIGHT_MOUSE) {
				//mouseOnPush = FL_RIGHT_MOUSE;
				mxmem = Fl::event_x();
				mymem = Fl::event_y();
				hamem = camera.horAngle;
				vamem = camera.verAngle;
				return 1;
			}
			return Fl_Gl_Window::handle(e);
			break;
		case FL_MOUSEWHEEL:
			{
				distPower += Fl::event_dy() * 0.2f;
				if (distPower < 0) {
					distPower = 0.0f;
				}
				else if (distPower > 4.5f) {
					distPower = 4.5f;
				}
				camera.SetDistance(pow(glm::e<float>(), distPower));
				redraw();
				return 1;
			}
			return Fl_Gl_Window::handle(e);
		case FL_DRAG:
			//if (mouseOnPush == FL_RIGHT_MOUSE) {
			if (Fl::event_button3()) {
				camera.SetAngles(hamem + (float)(Fl::event_x() - mxmem)*pixToAngle, vamem + (float)(Fl::event_y() - mymem)*pixToAngle);
				redraw();
				return 1;
			}
		default: // fallthrough
			return Fl_Gl_Window::handle(e);
		}
	}

	void drawCoordSystem() {
		glBegin(GL_LINES);
		{
			glColor3ub(255, 0, 0); glVertex3i(0, 0, 0); glVertex3i(1, 0, 0);
			glColor3ub(0, 255, 0); glVertex3i(0, 0, 0); glVertex3i(0, 1, 0);
			glColor3ub(0, 0, 255); glVertex3i(0, 0, 0); glVertex3i(0, 0, 1);
		} glEnd();
	}

	GuiSharedData* guidata;
	EditorCamera camera;
protected:
	int mxmem, mymem;
	float hamem, vamem;
	float distPower;
	glm::mat4 projMat;
	//int mouseOnPush;

	void ChangeViewPort(int w, int h, bool turnUpsideDown = false) {
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		//glLoadIdentity();
		//gluPerspective(40, (float)w / (float)h, 0.1, 150);
		projMat = glm::perspective(glm::radians(40.0f), (float)w / (float)h, 0.1f, 150.0f);
		glLoadMatrixf(projMat[0].data);
		if (turnUpsideDown)
			glScalef(1, -1, 1);
	}
};

class BrickHeapPreview : public GlPreview {
public:

	BrickHeapPreview(int X, int Y, int W, int H, GuiSharedData* gd, const char*L = 0) : GlPreview(X, Y, W, H, gd, L)
	{
		camera.SetAngles(1.0f, 0.5f);
		camera.SetDistance(3.0f);
		camera.SetUp(0, 0, 1);
		camera.SetFocusPoint(0.5f, 0.5f, 0.5f);
	}

	void draw() {
		ChangeViewPort(w(), h());
		glMatrixMode(GL_MODELVIEW);
		glm::mat4 mat;
		camera.CalcMatrix(mat);
		glLoadMatrixf(mat[0].data);

		glClearColor(192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_TEXTURE_2D);
		glColor3ub(255, 255, 255);
		DrawBrickHeap(guidata->brickheap, guidata->tex);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		drawCoordSystem();

		// DEBUG: CHECK FOR ERRORS
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr, "GLGETERROR=%d\n", (int)err);
		}
	}
};

void LerpCallback(void * guidata);

class LevelPreview : public GlPreview {
public:
	LevelData::Coordinate gridCenter;
	glm::vec3 gridUp, A, B, T1, T2, T3;

	Interpolator<glm::vec3> lerper;

	LevelData::Coordinate lastClic;

	bool tileMode;

	LevelPreview(int X, int Y, int W, int H, GuiSharedData* gd, const char*L = 0)
		: GlPreview(X, Y, W, H, gd, L)
	{
		camera.SetAngles(1.0f, 0.5f);
		camera.SetDistance(6.0f);
		camera.SetUp(0, 0, 1);
		camera.SetFocusPoint(0.5f, 0.5f, 0.5f);

		gridUp = glm::vec3(0, 0, 1);
		lerper.target = &camera.focuspoint;
		tileMode = false;
	}

	void glideToNewCenter() {
		lerper.Set(camera.focuspoint, gridCenter.asVec3() + glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 16);
		lerper.Begin();
		Fl::add_timeout((double)lerper.dt, LerpCallback, (void*)guidata);
	}

	void handlePick() {
		glm::mat4 cam;
		camera.CalcMatrix(cam);
		glm::vec4 vp(0, 0, w(), h());

		// the aspect ratio seems to be ignored by glm::unProject, wtf? so here's an horrible fix...
		float vpw = (float)w();
		float aspectRatio = vpw / (float)h();
		float mx = (float)Fl::event_x();
		mx = (mx - vpw * 0.5f) * aspectRatio + vpw * 0.5f;

		glm::vec3 mouse(mx, (float)(h() - Fl::event_y()), 1.0f);
		glm::vec3 worldMouse = glm::unProject(mouse, cam, projMat, vp);

		glm::vec3 plane_center = gridCenter.asVec3();// ((float)gridCenter.x, (float)gridCenter.y, (float)gridCenter.z);
		if (gridUp.x + gridUp.y + gridUp.z < 0.0f) {
			plane_center -= gridUp;
		}
		glm::vec3 pick;
		glm::vec3 rayDir = glm::normalize(worldMouse - camera.GetPosition());
		if (Fl::event_ctrl()) {
			if (Fl::event_clicks() == 1) { // double clic
				LevelData::RayCastResult res;
				if (guidata->leveldata->RayCast(camera.GetPosition(), rayDir, 70.0f, &res)) {
					/*A = res.intersection;

					const LevelData::Brick& brick = guidata->leveldata->GetBrick(res.coordinate)[res.brickIndex];
					brick.TransformTriangle(res.coordinate.asVec3(), res.triangleIndex, T1, T2, T3);

					glm::vec3 tn = glm::normalize(glm::cross(T2 - T1, T3 - T1));
					B = A + tn;
					redraw();*/
					
					gridCenter = res.coordinate;
					glideToNewCenter();
				}
			}
		}
		else if (tileMode) {
			LevelData::RayCastResult res;
			if (guidata->leveldata->RayCast(camera.GetPosition(), rayDir, 70.0f, &res)) {
				guidata->leveldata->UpdateBrickTile(res.coordinate, res.brickIndex, Fl::event_button2() ? -1 : guidata->cur_ts_id, guidata->cur_tile_x, guidata->cur_tile_y);
				redraw();
			}
		}
		else if (linePlaneIntersection(plane_center, gridUp, camera.GetPosition(), rayDir, true, &pick)) {
			LevelData::Coordinate c(
				gridUp.x != 0.0f ? gridCenter.x : (int)glm::floor(pick.x),
				gridUp.y != 0.0f ? gridCenter.y : (int)glm::floor(pick.y),
				gridUp.z != 0.0f ? gridCenter.z : (int)glm::floor(pick.z));
			if (c != lastClic) {
				lastClic = c;
				if (Fl::event_shift()) {
					if (Fl::event_button2()) {
						guidata->leveldata->ClearBrick(c);
					}
					else {
						guidata->leveldata->StackBrick(c, guidata->brickheap);
					}
				}
				else {
					gridCenter = c;
				}
				redraw();
			}
			else if (Fl::event_clicks() == 1) { // double clic
				glideToNewCenter();
			}
		}
	}

	int handle(int e) {
		switch (e) {
		case FL_HIDE:
			guidata->tex.Clear();
			break;
		case FL_FOCUS:
		case FL_UNFOCUS:
			return 1;
			break;
		case FL_KEYBOARD:
			if (Fl::event_key() == ' ') {
				if (gridUp.x == 1.0f) {
					gridUp = glm::vec3(-1.0f,0.0f,0.0f);
				} else if (gridUp.x == -1.0f) {
					gridUp = glm::vec3(0.0f, 1.0f, 0.0f);
				} else if (gridUp.y == 1.0f) {
					gridUp = glm::vec3(0.0f, -1.0f, 0.0f);
				} else if (gridUp.y == -1.0f) {
					gridUp = glm::vec3(0.0f, 0.0f, 1.0f);
				} else if (gridUp.z == 1.0f) {
					gridUp = glm::vec3(0.0f, 0.0f, -1.0f);
				} else if (gridUp.z == -1.0f) {
					gridUp = glm::vec3(1.0f, 0.0f, 0.0f);
				}
				redraw();
				return 1;
			}
			else if (Fl::event_ctrl()) {
				if (Fl::event_key() == 'c') {
					guidata->brickheap = guidata->leveldata->GetBrick(gridCenter);
					guidata->brickheappreview->redraw();
					return 1;
				}
				else if (Fl::event_key() == 'x') {
					guidata->brickheap = guidata->leveldata->GetBrick(gridCenter);
					guidata->leveldata->ClearBrick(gridCenter);
					guidata->brickheappreview->redraw();
					redraw();
					return 1;
				}
				else if (Fl::event_key() == 'v') {
					guidata->leveldata->StackBrick(gridCenter, guidata->brickheap);
					redraw();
					return 1;
				}
			}
			else if (Fl::event_key() == 't') {
				tileMode = !tileMode;
			}
			break;
		case FL_PUSH:
			take_focus();
			if (Fl::event_button() == FL_LEFT_MOUSE || Fl::event_button() == FL_MIDDLE_MOUSE) {
				handlePick();
				return 1;
			}
			break;
		case FL_DRAG:
			if (Fl::event_button1() || Fl::event_button2()) {
				handlePick();
				return 1;
			}
			break;
		case FL_MOUSEWHEEL:
			if (Fl::event_ctrl()) {
				gridCenter.x += Fl::event_dy() * (int)gridUp.x;
				gridCenter.y += Fl::event_dy() * (int)gridUp.y;
				gridCenter.z += Fl::event_dy() * (int)gridUp.z;
				redraw();
				return 1;
			}
			break;
		default:
			return GlPreview::handle(e);
		}
		return GlPreview::handle(e);
	}

	void draw() {
		ChangeViewPort(w(), h());

		glMatrixMode(GL_MODELVIEW);
		glm::mat4 mat;
		camera.CalcMatrix(mat);
		glLoadMatrixf(mat[0].data);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw the level
		glColor3ub(255, 255, 255);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		DrawLevelData(*guidata->leveldata, guidata->tex);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_TEXTURE_2D);


		//glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		// debug
		glColor3ub(255, 0, 255);
		glBegin(GL_LINES);
		glVertex3fv(&A[0]); glVertex3fv(&B[0]);
		glVertex3fv(&T1[0]); glVertex3fv(&T2[0]);
		glVertex3fv(&T2[0]); glVertex3fv(&T3[0]);
		glVertex3fv(&T3[0]); glVertex3fv(&T1[0]);
		glEnd();
		// grid
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
			for (int i = -range; i <= range+1; ++i) {
				glVertex3i(i, -range, 0); glVertex3i(i, range+1, 0);
				glVertex3i(-range, i, 0); glVertex3i(range+1, i, 0);
			}
		}
		glEnd();
		glPopMatrix();

		// highlighted position
		glColor3ub(255, 255, 0);
		glPushMatrix();

		glTranslatef((float)gridCenter.x + 0.5f, (float)gridCenter.y + 0.5f, (float)gridCenter.z + 0.5f);
		
		DrawGlWireCube(-0.6f, 0.6f);

		glPopMatrix();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		ChangeViewPort(100, 100);
		glMatrixMode(GL_MODELVIEW);
		EditorCamera cam = camera;
		cam.SetDistance(2.5f);
		cam.SetFocusPoint(0.0f, 0.0f, 0.0f);
		cam.CalcMatrix(mat);
		glLoadMatrixf(mat[0].data);
		drawCoordSystem();
	}
};

void LerpCallback(void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	if (gd->levelPreview->lerper.Step()) {
		Fl::remove_timeout(LerpCallback, guidata);
	}
	else {
		Fl::repeat_timeout((double)gd->levelPreview->lerper.tick, LerpCallback, guidata);
	}
	gd->levelPreview->redraw();
}

class BrickWindow : public GlPreview {
public:
	BrickWindow(int X, int Y, int W, int H, GuiSharedData* gd, const char*L = 0)
		: GlPreview(X, Y, W, H, gd, L)
	{
		camera.SetAngles(1.0f, 0.5f);
		camera.SetDistance(3.0f);
		camera.SetUp(0, 0, 1);
		camera.SetFocusPoint(0.5f, 0.5f, 0.5f);
	}

	void draw() {
		ChangeViewPort(w(), h());
		draw(guidata->leveldata->GetBrickData(guidata->cur_brick));
	}

	void draw(const LevelData::BrickData* brick) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glm::mat4 mat;
		camera.CalcMatrix(mat);
		glMultMatrixf(mat[0].data);

		glClearColor(192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glColor3f(0.5, 0.5, 0.5);
		DrawGlWireCube(0.0f, 1.0f);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glColor3ub(255, 255, 255);
		DrawBrick(brick);

		// DEBUG: CHECK FOR ERRORS
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr, "GLGETERROR=%d\n", (int)err);
		}
	}

	Fl_Image* TakeScreenshot(int w, int h, LevelData::BrickData* brick) {
		flush();
		ChangeViewPort(w, h, true);
		draw(brick);
		//swap_buffers();

		unsigned char* pixels = new unsigned char[4 * w*h];
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		ChangeViewPort(this->w(), this->h());
		//redraw();

		Fl_RGB_Image* image = new Fl_RGB_Image(pixels, w, h, 4);
		return image;
	}
};

class BrickPreview : public Fl_Group
{
public:
	BrickPreview(int X, int Y, int index, GuiSharedData* guidata) :
		Fl_Group(X,Y,100,100),
		index(index),
		guidata(guidata)
	{
		box(FL_UP_BOX);
		begin();

		myRemover = new Fl_Button(X + 85, Y, 15, 15, "@1+");
		myRemover->callback(RemoveBrickCallback, (void*)guidata);

		end();

		UpdateImagePreview();
	}

	~BrickPreview() {
		if (image()) {
			if (image()->data()) {
				delete[] (unsigned char*) image()->data()[0];
				//image()->data(0, 0);
			}
			delete image();
			image(0);
		}
	}

	void draw() {
		if (guidata->cur_brick == index) {
			color(FL_DARK2);
		}
		else {
			color(FL_GRAY);
		}
		Fl_Group::draw();
		if (image()) {
			image()->draw(x(), y());
		}
	}

	int handle(int event) {
		switch (event) {
		case FL_PUSH:
			if (Fl::event_button() == FL_LEFT_MOUSE) {

				// ignore for ourself if a child can take the push
				for (int i = children(); i--;) {
					if (Fl::event_inside(child(i))) {
						return Fl_Group::handle(event);
					}
				}

				// the code after this for loop may remove some widgets in the vertex or triangle editor
				// before the "changed" event can happen in their input fields
				// that's why I force the unfocus event so they can update first.
				for (int i = 0; i < guidata->vertexeditor->children(); ++i) {
					guidata->vertexeditor->child(i)->handle(FL_UNFOCUS);
				}

				// before changing brick, update the image preview
				BrickPreview* brother = (BrickPreview*)guidata->brickchooser->child(guidata->cur_brick);
				brother->UpdateImagePreview();

				guidata->cur_brick = index;
				guidata->brickchooser->redraw();
				SetCurrentBrickCallback(this, (void*)guidata);

				return 1;
			}
			// intentional fall through
		default:
			return Fl_Group::handle(event);
		}
	}

	void UpdateImagePreview()
	{
		if (image()) {
			if (image()->data()) {
				delete[] (unsigned char*) image()->data()[0];
				//image()->data(0,0);
			}
			delete image();
			image(0);
		}
		image(guidata->brickwindow->TakeScreenshot(w(), h(), guidata->leveldata->GetBrickData(index)));
		redraw();
	}

	int index;
	Fl_Button* myRemover;
	GuiSharedData* guidata;
};

class VertexField : public Fl_Float_Input {
public:
	VertexField(int X, int Y, int W, int H, const char* L = 0) : Fl_Float_Input(X,Y,W,H,L) {}

	int handle(int event) {
		if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
			GuiSharedData* gd = (GuiSharedData*)argument();
			int myVertex = gd->vertexeditor->find(this) / 5;
			HighlightVertex(gd, myVertex);
		}
		return Fl_Float_Input::handle(event);
	}
};

class TriangleField : public Fl_Int_Input {
public:
	TriangleField(int X, int Y, int W, int H, const char* L = 0) : Fl_Int_Input(X, Y, W, H, L) {}

	int handle(int event) {
		if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
			GuiSharedData* gd = (GuiSharedData*)argument();
			int myTriangle = gd->triangleeditor->find(this) / 3;
			HighlightTriangle(gd, myTriangle);
		}
		return Fl_Int_Input::handle(event);
	}
};

void OpenWindowCallback(Fl_Widget* caller, void* win) {
	((Fl_Window*)win)->show();
}

void LoadTilesetImageCallback(Fl_Widget*, void* tilesetmanagerdata) {
	/*Fl_File_Chooser chooser(".", "Image Files (*.{bmp,gif,jpg,png})", Fl_File_Chooser::SINGLE, "Select tileset");
	chooser.show();

	while (chooser.shown()){
		Fl::wait();
	}

	// Cancel?
	if (chooser.value() == NULL) {
		return;
	}*/

	Fl_Native_File_Chooser chooser;
	chooser.title("Load tileset image");
	chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser.filter("Image files\t*.{bmp,gif,jpg,png}");

	int res = chooser.show();
	if (res == -1 || res == 1) {
		return; // either cancel or error
	}

	TilesetManagerData* tmdt = (TilesetManagerData*)tilesetmanagerdata;
	//tmdt->box_filename->copy_label(chooser.value());
	//tmdt->tilesetpreview->image(Fl_Shared_Image::get(chooser.value()));
	tmdt->box_filename->copy_label(chooser.filename());
	tmdt->tilesetpreview->image(Fl_Shared_Image::get(chooser.filename()));

	tmdt->box_filename->redraw();
	tmdt->tilesetpreview->redraw();
	tmdt->tilesetpreview->redraw_label();
}

void LoadFileCallback(Fl_Widget * caller, void *guidata)
{
	Fl_Native_File_Chooser chooser;
	chooser.title("Load level");
	chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser.filter("Level format\t*.{lvl,nv}");

	int res = chooser.show();
	if (res == -1 || res == 1) {
		return; // either cancel or error
	}

	GuiSharedData* gd = (GuiSharedData*)guidata;
	if (gd->leveldata->Load(chooser.filename())) {
		// select the first object instance
		const GameData& instances = gd->leveldata->GetInstances();
		if (!instances.values.empty()) {
			gd->objdata->cur_instance = instances.values.begin()->first;
		}
		else {
			gd->objdata->cur_instance.clear();
		}

		AllReloadCallbacksCallback(caller, guidata);
	}
}

void SaveFileCallback(Fl_Widget * caller, void *guidata)
{
	Fl_Native_File_Chooser chooser;
	chooser.title("Save level");
	chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	chooser.filter("Level format\t*.{lvl,nv}");

	int res = chooser.show();
	if (res == -1 || res == 1) {
		return; // either cancel or error
	}

	GuiSharedData* gd = (GuiSharedData*)guidata;
	gd->leveldata->Save(chooser.filename());
}

void TilesetOkCallback(Fl_Widget* caller, void*tilebutton) {
	TileButton* tb = (TileButton*)tilebutton;

	TilesetManagerData* tmdt = tb->guidata->tmdata;
	tb->guidata->leveldata->UpdateTilesetData(
		tb->index,
		tmdt->box_filename->label(),
		(int)tmdt->tileset_ox->value(),
		(int)tmdt->tileset_oy->value(),
		(int)tmdt->tileset_px->value(),
		(int)tmdt->tileset_py->value(),
		(int)tmdt->tileset_tw->value(),
		(int)tmdt->tileset_th->value()
	);

	tb->guidata->tmdata->tileset_ok->window()->hide();
	ReloadTilesetDataCallback(caller, (void*)tb->guidata);
	ReloadTilechooserCallback(caller, (void*)tb->guidata);
}

void LoadToHeapCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);
	if (brick) {
		gd->brickheap.clear();
		LevelData::Brick b(*gd->leveldata);
		b.brickdata = brick;
		gd->brickheap.push_back(b);
		gd->brickheappreview->redraw();
	}
}

void EditTilesetCallback(Fl_Widget* caller, void* tilebutton) {
	TileButton* tb = (TileButton*)tilebutton;

	TilesetManagerData* tmdt = tb->guidata->tmdata;

	const LevelData::TilesetData* tsdt = tb->guidata->leveldata->GetTilesetData(tb->index);

	tmdt->box_filename->label(tsdt->file.c_str());
	tmdt->tilesetpreview->image(Fl_Shared_Image::get(tsdt->file.c_str()));
	tmdt->tileset_ox->value(tsdt->ox);
	tmdt->tileset_oy->value(tsdt->oy);
	tmdt->tileset_px->value(tsdt->px);
	tmdt->tileset_py->value(tsdt->py);
	tmdt->tileset_tw->value(tsdt->tw);
	tmdt->tileset_th->value(tsdt->th);

	tmdt->tileset_ok->callback(TilesetOkCallback,tilebutton);

	tb->guidata->tmdata->tileset_ok->window()->show();
}

void RedrawWidgetCallback(Fl_Widget*, void* widgetToRedraw) {
	Fl_Widget* w = (Fl_Widget*)widgetToRedraw;
	w->redraw();
	w->redraw_label();
}

void ReloadLevelPreviewCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	gd->levelPreview->redraw();
}

void SetMenuItem(Fl_Menu_& menu, const char* l) {
	Fl_Menu_Item* item = (Fl_Menu_Item*)menu.find_item(l);
	if (item) {
		item->set();
	}
}

void ReloadTilesetDataCallback(Fl_Widget*, void* guidata) {
	GuiSharedData* gd = (GuiSharedData*)guidata;
	if (gd->scroller->window()->shown()) {

		int idstop = gd->scroller->children() - 3;
		for (int i = 0; i < idstop; ++i) {
			gd->scroller->remove(0);
		}

		const LevelData::TilesetData* tsdata = gd->leveldata->GetTilesetData(0);
		for (unsigned int i = 0; tsdata; ++i) {
			TileButton* img_button = new TileButton(i, gd);
			img_button->image = Fl_Shared_Image::get(tsdata->file.c_str());
			tsdata = gd->leveldata->GetTilesetData(i + 1);
			gd->scroller->insert(*img_button, gd->scroller->children() - 3);
		}

		gd->scroller->resize(gd->scroller->x(), gd->scroller->y(), gd->scroller->w(), gd->scroller->h());
		gd->scroller->redraw();
	}
}

void ReloadTilechooserCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	if (gd->tilechooser->window()->shown()) {
		gd->tilechooser->Reload();
	}
}

void ReloadVertexEditorCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);

	int nbWidgetLines = (gd->vertexeditor->children() - 2) / 5;
	int vcount = brick ? brick->GetVertexCount() : 0;
		
	if (nbWidgetLines > vcount) {
		// need to remove some widgets
		for (int i = vcount * 5; i < nbWidgetLines * 5; ++i) {
			gd->vertexeditor->remove(vcount * 5);
		}
	}
	else if (nbWidgetLines < vcount) {
		// need to add some widgets
		for (int i = nbWidgetLines; i < vcount; ++i) {
			for (int j = 0; j < 5; ++j) {
				Fl_Float_Input* fi = new VertexField(10 + j * 40, 160 + 25 * i, 40, 25);
				fi->box(FL_BORDER_BOX);
				fi->callback(EditVertexCallback, guidata);
				gd->vertexeditor->insert(*fi, i*5+j);
			}
		}
	}

	// update fields
	for (int i = 0; i < vcount; ++i) {
		for (int j = 0; j < 5; ++j) {
			Fl_Float_Input* fi = (Fl_Float_Input*)gd->vertexeditor->child(i * 5 + j);
			fi->value(valToStr(brick->GetVertex(i)->values[j]).c_str());
		}
	}

	gd->vertexeditor->redraw();
	gd->brickwindow->redraw();
}

void ReloadTriangleEditorCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);

	int nbWidgetLines = (gd->triangleeditor->children() - 2) / 3;
	int tcount = brick ? brick->GetTriangleList().size() / 3 : 0;

	if (nbWidgetLines > tcount) {
		// need to remove some widgets
		for (int i = tcount * 3; i < nbWidgetLines * 3; ++i) {
			gd->triangleeditor->remove(tcount * 3);
		}
	}
	else if (nbWidgetLines < tcount) {
		// need to add some widgets
		for (int i = nbWidgetLines; i < tcount; ++i) {
			for (int j = 0; j < 3; ++j) {
				Fl_Int_Input* ii = new TriangleField(230 + j * 20, 130 + 20 * i, 20, 20);
				ii->box(FL_FLAT_BOX);
				ii->callback(EditTriangleCallback, guidata);
				gd->triangleeditor->insert(*ii, i * 3 + j);
			}
		}
	}

	// update fields
	for (int i = 0; i < tcount; ++i) {
		for (int j = 0; j < 3; ++j) {
			Fl_Int_Input* ii = (Fl_Int_Input*)gd->triangleeditor->child(i * 3 + j);
			ii->value(valToStr(brick->GetTriangleList()[i * 3 + j]).c_str());
		}
	}

	gd->triangleeditor->redraw();
	gd->brickwindow->redraw();
}

void ReloadBrickChooserCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;

	while (gd->brickchooser->children() > 3) {
		gd->brickchooser->remove(0);
	}

	Fl_Widget* addButton = gd->brickchooser->child(0);
	int bc = gd->leveldata->GetBrickDataCount();

	for (int i = 0; i < bc; ++i) {
		gd->brickchooser->insert(*(new BrickPreview(i * 100, 0, i, gd)), addButton);
	}

	addButton->position(bc * 100, 0);

	gd->brickchooser->redraw();
	gd->brickwindow->redraw();
}

void ReloadBrickEditorCallback(Fl_Widget * caller, void * guidata) {
	GuiSharedData* gd = (GuiSharedData*)guidata;
	if (gd->brickchooser->window()->shown()) {
		ReloadVertexEditorCallback(caller, guidata);
		ReloadTriangleEditorCallback(caller, guidata);
		ReloadBrickChooserCallback(caller, guidata);
	}
}

#define JustInsert( objvar, wid ) { \
ObjectVariableWidgetInfo info; \
info.type = ObjectVariableWidgetInfo::none; \
info.widget = wid; \
objvar.widgets.push_back(info); \
}

#define FillInfoAndInsert( objvar, wid, typ, val ) { \
ObjectVariableWidgetInfo info;\
info.type = ObjectVariableWidgetInfo::t ## typ; \
info.v ## typ = val; \
info.widget = wid; \
objvar.widgets.push_back(info); \
wid->callback(EditObjFieldCallback, (void*)&objvar); \
}


void EditObjFieldCallback(Fl_Widget * input, void * objvar) {
	ObjectVariable* ov = (ObjectVariable*)objvar;
	std::string val;
	for (unsigned int i = 0; i < ov->widgets.size(); ++i) {
		val.append(((Fl_Input*)ov->widgets[i].widget)->value());
		val.append(1, ' ');
	}
	ov->leveldata->UpdateInstance(ov->field, val);
}

void RenameObjCallback(Fl_Widget * widget, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	if (!gd->objdata->cur_instance.empty()) {
		Fl_Input* input = (Fl_Input*)widget;
		gd->leveldata->RenameInstance(gd->objdata->cur_instance, input->value());
		gd->objdata->cur_instance = input->value();
	}
}

void SelectObjCallback(Fl_Widget * input, void * guidata)
{
	Fl_Select_Browser* browser = (Fl_Select_Browser*)input;
	if (browser->value()) {
		GuiSharedData* gd = (GuiSharedData*)guidata;
		gd->objdata->cur_instance = browser->text(browser->value());
		ReloadObjCallback(input, guidata);
	}
}

void ReloadObjNameLocRotScaleCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	int x = gd->objdata->tabs->x() + 10;

	if (!gd->objdata->cur_instance.empty()) {
		const std::string& instance = gd->objdata->cur_instance;
		gd->objdata->type_instance->value(gd->leveldata->GetInstances().GetType(instance).c_str());
		gd->objdata->nom_instance->value(instance.c_str());
		const GameData& gmd = gd->leveldata->GetInstances().GetValue<GameData>(instance);
		if (gmd.HasValue<GameData>("transform")) {
			std::string trname = instance;
			trname.append(".transform");
			const GameData& tranform = gmd.GetValue<GameData>("transform");
			{
				const glm::vec3& loc = tranform.GetValue<glm::vec3>("loc");
				gd->objdata->loc_scale->begin();
				Fl_Float_Input* pos_x = new Fl_Float_Input(x + 90, 110, 40, 25, "Loc X");
				Fl_Float_Input* pos_y = new Fl_Float_Input(x + 90, 150, 40, 25, "Loc Y");
				Fl_Float_Input* pos_z = new Fl_Float_Input(x + 90, 190, 40, 25, "Loc Z");
				gd->objdata->loc_scale->end();
				gd->objdata->var_content.push_back(ObjectVariable());
				ObjectVariable& var = gd->objdata->var_content.back();
				var.leveldata = gd->leveldata;
				var.field = trname; var.field.append(".loc");
				FillInfoAndInsert(var, pos_x, float, &loc.x);
				FillInfoAndInsert(var, pos_y, float, &loc.y);
				FillInfoAndInsert(var, pos_z, float, &loc.z);
			}

			{
				const glm::vec3& scale = tranform.GetValue<glm::vec3>("scale");
				gd->objdata->loc_scale->begin();
				Fl_Float_Input* scale_x = new Fl_Float_Input(x + 210, 110, 40, 25, "Scale X");
				Fl_Float_Input* scale_y = new Fl_Float_Input(x + 210, 150, 40, 25, "Scale Y");
				Fl_Float_Input* scale_z = new Fl_Float_Input(x + 210, 190, 40, 25, "Scale Z");
				gd->objdata->loc_scale->end();
				gd->objdata->var_content.push_back(ObjectVariable());
				ObjectVariable& var = gd->objdata->var_content.back();
				var.leveldata = gd->leveldata;
				var.field = trname; var.field.append(".scale");
				FillInfoAndInsert(var, scale_x, float, &scale.x);
				FillInfoAndInsert(var, scale_y, float, &scale.y);
				FillInfoAndInsert(var, scale_z, float, &scale.z);
			}

			{
				const glm::quat& rot = tranform.GetValue<glm::quat>("rot");
				gd->objdata->rotation_quat->begin();
				Fl_Float_Input* quat_x = new Fl_Float_Input(x + 90, 110, 40, 25, "Quat X");
				Fl_Float_Input* quat_y = new Fl_Float_Input(x + 90, 150, 40, 25, "Quat Y");
				Fl_Float_Input* quat_z = new Fl_Float_Input(x + 90, 190, 40, 25, "Quat Z");
				Fl_Float_Input* quat_w = new Fl_Float_Input(x + 210, 110, 40, 25, "Quat W");
				gd->objdata->rotation_quat->end();
				gd->objdata->loc_scale->end();
				gd->objdata->var_content.push_back(ObjectVariable());
				ObjectVariable& var = gd->objdata->var_content.back();
				var.leveldata = gd->leveldata;
				var.field = trname; var.field.append(".rot");
				FillInfoAndInsert(var, quat_x, float, &rot.x);
				FillInfoAndInsert(var, quat_y, float, &rot.y);
				FillInfoAndInsert(var, quat_z, float, &rot.z);
				FillInfoAndInsert(var, quat_w, float, &rot.w);
			}

			//Fl_Float_Input* eul_x = new Fl_Float_Input(160, 110, 40, 25, "Euler X");
			//Fl_Float_Input* eul_y = new Fl_Float_Input(160, 150, 40, 25, "Euler Y");
			//Fl_Float_Input* eul_z = new Fl_Float_Input(160, 190, 40, 25, "Euler Z");
		}
	}
}

void ReloadObjVariablesRecursive(int &y, const std::string& labelstr, const std::string& field, const GameData& gmd, GuiSharedData* gd) {
	Fl_Scroll* sc = gd->objdata->variables;
	int x = sc->x() + 10;
	int w = sc->w() - 40;
	int h = 20;
	int ystep = h + 20;
	int w2 = 50;
	std::list<ObjectVariable>& vars = gd->objdata->var_content;
	for (GameData::Values::const_iterator it = gmd.values.begin(); it != gmd.values.end(); ++it) {
		if (it->first != "transform") {
			vars.push_back(ObjectVariable());
			ObjectVariable& var = vars.back();
			var.leveldata = gd->leveldata;
			var.field = field;
			var.field.append(1, '.').append(it->first);
			const std::string& type = it->second->Type();

			std::string mylabelstr = labelstr;
			if (!mylabelstr.empty()) { mylabelstr.append(1, '.'); }
			mylabelstr.append(it->first);
			
			if (gmd.IsComposite(it->first)) {
				const GameData& gmd2 = gmd.GetValue<GameData>(it->first);
				//y -= ystep;
				Fl_Box* box = new Fl_Box(x - 1, y - ystep / 2, 1, h);
				sc->add(box);
				ReloadObjVariablesRecursive(y, mylabelstr, var.field, gmd2, gd);
				JustInsert(var, box);
				//var.widgets.push_back(new Fl_Box(x - 1, y, 1, h));
			}
			else if (type == "string") {
				Fl_Input* str = new Fl_Input(x, y, w, h);
				str->value(gmd.GetValue<std::string>(it->first).c_str());
				sc->add(str);
				//var.widgets.push_back(str);
				FillInfoAndInsert(var, str, string, &gmd.GetValue<std::string>(it->first))
				y += ystep;
			}
			else if (type == "uint" || type == "int") {
				Fl_Int_Input* ipt = new Fl_Int_Input(x, y, w2, h);
				if (type == "uint") {
					//ipt->value(valToStr(gmd.GetValue<unsigned int>(it->first)).c_str());
					FillInfoAndInsert(var, ipt, uint, &gmd.GetValue<unsigned int>(it->first))
				}
				else if(type == "int") {
					//ipt->value(valToStr(gmd.GetValue<int>(it->first)).c_str());
					FillInfoAndInsert(var, ipt, int, &gmd.GetValue<int>(it->first))
				}
				sc->add(ipt);
				//var.widgets.push_back(ipt);
			}
			else if (type == "float" || type == "double") {
				Fl_Float_Input* fpt = new Fl_Float_Input(x, y, w2, h);
				if (type == "float") {
					//fpt->value(valToStr(gmd.GetValue<float>(it->first)).c_str());
					FillInfoAndInsert(var, fpt, float, &gmd.GetValue<float>(it->first))
				}
				else if (type == "double") {
					//fpt->value(valToStr(gmd.GetValue<double>(it->first)).c_str());
					FillInfoAndInsert(var, fpt, double, &gmd.GetValue<double>(it->first))
				}
				sc->add(fpt);
				//var.widgets.push_back(fpt);
			}
			else if (type == "vec3" || type == "vec4" || type == "quat") {
				unsigned int count = (type == "vec3") ? 3 : 4;
				int X = x;
				for (unsigned int i = 0; i < count; ++i) {
					Fl_Float_Input* fpt = new Fl_Float_Input(X, y, w2, h);
					//if (type == "vec3") { fpt->value(valToStr(gmd.GetValue<glm::vec3>(it->first)[i]).c_str()); }
					//if (type == "vec4") { fpt->value(valToStr(gmd.GetValue<glm::vec4>(it->first)[i]).c_str()); }
					//if (type == "quat") { fpt->value(valToStr(gmd.GetValue<glm::quat>(it->first)[i]).c_str()); }
					if (type == "vec3") { FillInfoAndInsert(var, fpt, float, &gmd.GetValue<glm::vec3>(it->first)[i]) }
					if (type == "vec4") { FillInfoAndInsert(var, fpt, float, &gmd.GetValue<glm::vec4>(it->first)[i]) }
					if (type == "quat") { FillInfoAndInsert(var, fpt, float, &gmd.GetValue<glm::quat>(it->first)[i]) }
					sc->add(fpt);
					//var.widgets.push_back(fpt);
					X += w2 + 15;
				}
			}
			else {
				Fl_Box* b1 = new Fl_Box(x - 1, y, 1, h);
				Fl_Box* b2 = new Fl_Box(x, y, w, h, "Type unsupported!");
				//var.widgets.push_back(b1);
				//var.widgets.push_back(b2);
				JustInsert(var, b1);
				JustInsert(var, b2);
				sc->add(b1);
				sc->add(b2);
			}
			y += ystep;

			var.labelstr = mylabelstr.append(" :");
			var.widgets[0].widget->label(var.labelstr.c_str());
			var.widgets[0].widget->align(FL_ALIGN_TOP_LEFT);
		}
	}
}

void ReloadObjVariablesCallback(Fl_Widget *, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;

	if (!gd->objdata->cur_instance.empty()) {
		const GameData& gmd = gd->leveldata->GetInstances().GetValue<GameData>(gd->objdata->cur_instance);
		int y = gd->objdata->variables->y() + 20;
		ReloadObjVariablesRecursive(y, "", gd->objdata->cur_instance, gmd, gd);
	}
}

void RefreshObjContent(std::list<ObjectVariable>& content) {
	for (std::list<ObjectVariable>::iterator it = content.begin(); it != content.end(); ++it) {
		ObjectVariable& var = *it;
		for (unsigned int j = 0; j < var.widgets.size(); ++j) {
			ObjectVariableWidgetInfo& info = var.widgets[j];
			switch (info.type) {
			case ObjectVariableWidgetInfo::tint:
				((Fl_Int_Input*)info.widget)->value(valToStr(*info.vint).c_str());
				break;
			case ObjectVariableWidgetInfo::tuint:
				((Fl_Int_Input*)info.widget)->value(valToStr(*info.vuint).c_str());
				break;
			case ObjectVariableWidgetInfo::tfloat:
				((Fl_Float_Input*)info.widget)->value(valToStr(*info.vfloat).c_str());
				break;
			case ObjectVariableWidgetInfo::tdouble:
				((Fl_Float_Input*)info.widget)->value(valToStr(*info.vdouble).c_str());
				break;
			case ObjectVariableWidgetInfo::tstring:
				((Fl_Input*)info.widget)->value(info.vstring->c_str());
				break;

			default:break;
			}
		}
	}
}

void ReloadObjCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;

	const GameData& instances = gd->leveldata->GetInstances();

	gd->objdata->browser->clear();
	int line = 1;
	for (GameData::Values::const_iterator it = instances.values.begin(); it != instances.values.end(); ++it) {
		gd->objdata->browser->add(it->first.c_str(), 0);
		if (gd->objdata->cur_instance == it->first.c_str()) {
			gd->objdata->browser->value(line);
		}
		++line;
	}
	gd->objdata->browser->add("", 0);
	if (gd->objdata->cur_instance.empty()) {
		gd->objdata->browser->value(line);
	}

	std::string newInstance;
	if (!gd->objdata->cur_instance.empty()) {
		if (!gd->leveldata->GetInstances().HasField(gd->objdata->cur_instance)) {
			gd->objdata->cur_instance.clear();
		}
		newInstance = gd->objdata->cur_instance;
	}

	bool instanceIsDifferent = (newInstance != gd->objdata->lastInstance);
	if (instanceIsDifferent) {
		for (std::list<ObjectVariable>::iterator it = gd->objdata->var_content.begin(); it != gd->objdata->var_content.end(); ++it) {
			ObjectVariable& var = *it;
			for (unsigned int j = 0; j < var.widgets.size(); ++j) {
				delete var.widgets[j].widget;
			}
		}
		gd->objdata->var_content.clear();
	}

	if (!gd->objdata->cur_instance.empty()) {
		if (instanceIsDifferent) {
			ReloadObjNameLocRotScaleCallback(caller, guidata);
			ReloadObjVariablesCallback(caller, guidata);
		}
		RefreshObjContent(gd->objdata->var_content);
	}

	gd->objdata->type_instance->parent()->redraw();
	gd->objdata->lastInstance = newInstance;
}

void AllReloadCallbacksCallback(Fl_Widget * caller, void * reloaddata)
{
	ReloadLevelPreviewCallback(caller, reloaddata);
	ReloadTilesetDataCallback(caller, reloaddata);
	ReloadTilechooserCallback(caller, reloaddata);
	ReloadBrickEditorCallback(caller, reloaddata);
	ReloadObjCallback(caller, reloaddata);
}

void AddTileButtonCallback(Fl_Widget* caller, void* guidata) {
	/*MyThumbnailViewer* scroller = (MyThumbnailViewer*)(((void**)args)[1]);
	Fl_Window* win = (Fl_Window*)(((void**)args)[2]);

	TileButton* tb = new TileButton(10, 10);
	tb->callback(*win, *scroller);

	scroller->insert(*tb, caller);

	scroller->resize(scroller->x(), scroller->y(), scroller->w(), scroller->h());
	scroller->redraw();*/

	GuiSharedData* rd = (GuiSharedData*)guidata;

	rd->leveldata->AddTilesetData();
	ReloadTilesetDataCallback(caller, guidata);
}

void RemoveTileButtonCallback(Fl_Widget* caller, void* reloaddata) {
	TileButton* tb = (TileButton*)reloaddata;
	
	tb->guidata->leveldata->RemoveTilesetData(tb->index);
	ReloadTilesetDataCallback(caller, (void*)tb->guidata);
	if (tb->index == tb->guidata->cur_ts_id) {
		tb->guidata->cur_ts_id = 0;
		ReloadTilechooserCallback(caller, (void*)tb->guidata);
	}
	else if (tb->index < tb->guidata->cur_ts_id) {
		--tb->guidata->cur_ts_id;
	}
}

void EditVertexCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	Fl_Scroll* ve = gd->vertexeditor;
	int line = ve->find(caller) / 5;
	gd->leveldata->GetBrickData(gd->cur_brick)->UpdateVertex(
		line,
		strToVal<float>(((Fl_Float_Input*)ve->child(line * 5))->value()),
		strToVal<float>(((Fl_Float_Input*)ve->child(line * 5 + 1))->value()),
		strToVal<float>(((Fl_Float_Input*)ve->child(line * 5 + 2))->value()),
		strToVal<float>(((Fl_Float_Input*)ve->child(line * 5 + 3))->value()), 
		strToVal<float>(((Fl_Float_Input*)ve->child(line * 5 + 4))->value()));

	gd->brickwindow->redraw();
}

void AddVertexCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);
	if (brick) {
		brick->AddVertex();
		ReloadVertexEditorCallback(caller, guidata);
	}
}

void RemoveVertexCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);
	if (brick && brick->RemoveVertex(gd->cur_vertex)) {
		ReloadVertexEditorCallback(caller, guidata);
	}
}

void EditTriangleCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);
	if (brick) {
		std::vector<int> trlist;
		for (int i = 0; i < gd->triangleeditor->children() - 2; ++i) {
			trlist.push_back(strToVal<int>(((Fl_Int_Input*)gd->triangleeditor->child(i))->value()));
		}
		brick->UpdateTriangleList(trlist);
		gd->brickwindow->redraw();
	}
}

void AddTriangleCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);
	if (brick) {
		std::vector<int> trlist = brick->GetTriangleList();

		trlist.insert(trlist.begin() + (gd->cur_triangle * 3), 3, 0);

		brick->UpdateTriangleList(trlist);
		ReloadTriangleEditorCallback(caller, guidata);
	}
}

void RemoveTriangleCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	LevelData::BrickData* brick = gd->leveldata->GetBrickData(gd->cur_brick);
	if (brick) {
		std::vector<int> trlist = brick->GetTriangleList();

		trlist.erase(trlist.begin() + (gd->cur_triangle * 3), trlist.begin() + ((gd->cur_triangle + 1) * 3));

		brick->UpdateTriangleList(trlist);
		ReloadTriangleEditorCallback(caller, guidata);
	}
}

void AddBrickCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	gd->brickchooser->insert(*(new BrickPreview(caller->x(), caller->y(), gd->leveldata->GetBrickDataCount(), gd)), caller);
	caller->position(caller->x() + 100, caller->y());
	gd->leveldata->AddBrickData();
	gd->brickchooser->redraw();
}

void RemoveBrickCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	BrickPreview* bp = (BrickPreview*)caller->parent();
	gd->leveldata->RemoveBrickData(bp->index);
	ReloadBrickChooserCallback(caller, guidata);
	ReloadVertexEditorCallback(caller, guidata);
	ReloadTriangleEditorCallback(caller, guidata);
}

void SetCurrentBrickCallback(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	ReloadVertexEditorCallback(caller, guidata);
	ReloadTriangleEditorCallback(caller, guidata);
}

void UndoCallback(Fl_Widget* caller, void* guidata) {
	GuiSharedData* rd = (GuiSharedData*)guidata;

	if (rd->leveldata->Undo()) {
		AllReloadCallbacksCallback(caller, guidata);
	}
}

void RedoCallback(Fl_Widget* caller, void* guidata) {
	GuiSharedData* rd = (GuiSharedData*)guidata;

	if (rd->leveldata->Redo()) {
		AllReloadCallbacksCallback(caller, guidata);
	}
}

typedef std::vector<void*> MultiArg;
void AddInMultiArg(MultiArg& multiarg, Fl_Callback callback, void* arg) {
	multiarg.push_back(callback);
	multiarg.push_back(arg);
}

void MultiCallback(Fl_Widget* caller, void* multiarg) {
	MultiArg* ma = (MultiArg*) multiarg;

	for (unsigned int i = 0; i < ma->size(); i+=2) {
		void* arg = 0;
		if (i + 1 < ma->size()) {
			arg = ma->at(i + 1);
		}

		Fl_Callback* cb = (Fl_Callback*)ma->at(i);
		cb(caller, arg);
	}
}

void BrickHeapNegRotX(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	for (unsigned int i = 0; i < gd->brickheap.size(); ++i) {
		gd->brickheap[i].matrix.RotXNeg();
	}
	gd->brickheappreview->redraw();
}

void BrickHeapPosRotX(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	for (unsigned int i = 0; i < gd->brickheap.size(); ++i) {
		gd->brickheap[i].matrix.RotXPos();
	}
	gd->brickheappreview->redraw();
}

void BrickHeapNegRotY(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	for (unsigned int i = 0; i < gd->brickheap.size(); ++i) {
		gd->brickheap[i].matrix.RotYNeg();
	}
	gd->brickheappreview->redraw();
}

void BrickHeapPosRotY(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	for (unsigned int i = 0; i < gd->brickheap.size(); ++i) {
		gd->brickheap[i].matrix.RotYPos();
	}
	gd->brickheappreview->redraw();
}

void BrickHeapNegRotZ(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	for (unsigned int i = 0; i < gd->brickheap.size(); ++i) {
		gd->brickheap[i].matrix.RotZNeg();
	}
	gd->brickheappreview->redraw();
}

void BrickHeapPosRotZ(Fl_Widget * caller, void * guidata)
{
	GuiSharedData* gd = (GuiSharedData*)guidata;
	for (unsigned int i = 0; i < gd->brickheap.size(); ++i) {
		gd->brickheap[i].matrix.RotZPos();
	}
	gd->brickheappreview->redraw();
}

void HighlightVertex(GuiSharedData * gd, int vertid)
{
	if (gd->cur_vertex != vertid && gd->cur_vertex * 5 - 2 < gd->vertexeditor->children()) {
		for (int i = 0; i < 5; ++i) {
			Fl_Widget* old = gd->vertexeditor->child(gd->cur_vertex * 5 + i);
			Fl_Widget* neo = gd->vertexeditor->child(vertid * 5 + i);
			old->color(FL_WHITE);
			neo->color(FL_LIGHT2);
			old->redraw();
			neo->redraw();
		}
	}
	gd->cur_vertex = vertid;
}

void HighlightTriangle(GuiSharedData * gd, int triid)
{
	if (gd->cur_triangle != triid && gd->cur_triangle * 3 - 2 < gd->triangleeditor->children()) {
		for (int i = 0; i < 3; ++i) {
			Fl_Widget* old = gd->triangleeditor->child(gd->cur_triangle * 3 + i);
			Fl_Widget* neo = gd->triangleeditor->child(triid * 3 + i);
			old->color(FL_WHITE);
			neo->color(FL_LIGHT2);
			old->redraw();
			neo->redraw();
		}
	}
	gd->cur_triangle = triid;
}

void drawRectangle(Fl_Color col) {
	fl_color(col);
	fl_begin_polygon();
	fl_vertex(-1, -0.5);
	fl_vertex(1, -0.5);
	fl_vertex(1, 0.5);
	fl_vertex(-1, 0.5);
	fl_end_polygon();
	
	fl_color(fl_darker(col));
	fl_begin_loop();
	fl_vertex(-1, -0.5);
	fl_vertex(1, -0.5);
	fl_vertex(1, 0.5);
	fl_vertex(-1, 0.5);
	fl_end_loop();
}

struct Gui::GuiInternal {
	LevelData leveldata;
	GuiSharedData guidata;
	TilesetManagerData tsdt;
	ObjectData objdata;
	
	std::vector<Fl_Widget*> widgets;
	std::vector<MultiArg*> multiArgs;

	template<class T>
	T& Add(T* t) {
		widgets.push_back((Fl_Widget*)t);
		return *t;
	}
};

void Gui::Send(Gui::Message::Enum msg) {
	Fl::awake((void*)msg);
}

bool Gui::HandleMessage(GuiContext* context, void* msg) {
	Gui::Message::Enum message = (Gui::Message::Enum)(int)msg;

	switch (message) {
	case Gui::Message::openMainWindow:
		context->mainWindow->show();
		break;
	case Gui::Message::quit:
		return false;
		break;
	default:
		break;
	}
	return true;
}

Gui::GuiContext* Gui::Load() {
	fl_register_images();

	GuiInternal* itrnl = new GuiInternal;

	fl_add_symbol("-", drawRectangle, 1);

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& object_prop = itrnl->Add(new Fl_Window(500, 360, "Objet properties"));
	object_prop.begin();

	Fl_Select_Browser& obj_browser = itrnl->Add(new Fl_Select_Browser(10, 10, 170, 340));
	obj_browser.callback(SelectObjCallback, &itrnl->guidata);
	int x = obj_browser.x() + obj_browser.w() + 10;

	Fl_Input& type_instance = itrnl->Add(new Fl_Input(x, 10, 120, 25));
	type_instance.value("Nom de l'objet");
	type_instance.color(FL_GRAY);

	Fl_Input& nom_instance = itrnl->Add(new Fl_Input(x, 40, 120, 25));
	nom_instance.value("Nom de l'instance");
	nom_instance.callback(RenameObjCallback, &itrnl->guidata);

	Fl_Tabs& tabs = itrnl->Add(new Fl_Tabs(x, 75, 300, 180));
	tabs.begin();

	Fl_Group& loc_scale = itrnl->Add(new Fl_Group(x, 100, 300, 160, "Loc Scale"));
	loc_scale.end();

	//Fl_Group& rotation_euler = itrnl->Add(new Fl_Group(10, 100, 300, 130, "Euler"));
	//Fl_Float_Input& eul_x = itrnl->Add(new Fl_Float_Input(160, 110, 40, 25, "Euler X"));
	//Fl_Float_Input& eul_y = itrnl->Add(new Fl_Float_Input(160, 150, 40, 25, "Euler Y"));
	//Fl_Float_Input& eul_z = itrnl->Add(new Fl_Float_Input(160, 190, 40, 25, "Euler Z"));
	//rotation_euler.end();

	Fl_Group& rotation_quat = itrnl->Add(new Fl_Group(x, 100, 300, 160, "Quat"));
	Fl_Button& quat_unit = itrnl->Add(new Fl_Button(x + 210, 150, 40, 25, "Unit"));
	rotation_quat.end();

	Fl_Group& var_group = itrnl->Add(new Fl_Group(x, 100, 300, 160, "Variables"));
	Fl_Scroll& variables = itrnl->Add(new Fl_Scroll(x+1, 101, 298, 153));
	Fl_Box& vartop = itrnl->Add(new Fl_Box(x, 100, 1, 1));
	variables.end();
	var_group.end();

	tabs.end();

	object_prop.end();

	itrnl->objdata.cur_instance.clear();
	itrnl->objdata.type_instance = &type_instance;
	itrnl->objdata.nom_instance = &nom_instance;
	itrnl->objdata.tabs = &tabs;
	itrnl->objdata.loc_scale = &loc_scale;
	//itrnl->objdata.rotation_euler = &rotation_euler;
	itrnl->objdata.rotation_quat = &rotation_quat;
	itrnl->objdata.variables = &variables;
	itrnl->objdata.browser = &obj_browser;

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& tileset_win = itrnl->Add(new Fl_Window(640, 480, "Tileset properties"));
	tileset_win.begin();

	//TilesetManagerData tmdata;

	Fl_Button& btn_load = itrnl->Add(new Fl_Button(5, 5, 50, 30, "@fileopen"));
	Fl_Box& box_filename = itrnl->Add(new Fl_Box(60, 5, 5, 30));
	box_filename.label("...");
	box_filename.align(FL_ALIGN_RIGHT);
	btn_load.callback(LoadTilesetImageCallback, (void*)&itrnl->tsdt);

	TilesetPreview& tilesetpreview = itrnl->Add(new TilesetPreview(5, 80, &itrnl->tsdt));

	Fl_Value_Input& tileset_ox = itrnl->Add(new Fl_Value_Input(30, 40, 40, 25, "OX")); tileset_ox.callback(RedrawWidgetCallback, (void*)&tilesetpreview);
	Fl_Value_Input& tileset_oy = itrnl->Add(new Fl_Value_Input(110, 40, 40, 25, "OY")); tileset_oy.callback(RedrawWidgetCallback, (void*)&tilesetpreview);
	Fl_Value_Input& tileset_px = itrnl->Add(new Fl_Value_Input(190, 40, 40, 25, "PX")); tileset_px.callback(RedrawWidgetCallback, (void*)&tilesetpreview);
	Fl_Value_Input& tileset_py = itrnl->Add(new Fl_Value_Input(270, 40, 40, 25, "PY")); tileset_py.callback(RedrawWidgetCallback, (void*)&tilesetpreview);
	Fl_Value_Input& tileset_tw = itrnl->Add(new Fl_Value_Input(350, 40, 40, 25, "TW")); tileset_tw.callback(RedrawWidgetCallback, (void*)&tilesetpreview);
	Fl_Value_Input& tileset_th = itrnl->Add(new Fl_Value_Input(430, 40, 40, 25, "TH")); tileset_th.callback(RedrawWidgetCallback, (void*)&tilesetpreview);

	Fl_Button& tileset_ok = itrnl->Add(new Fl_Button(500, 40, 30, 30, "OK"));

	Fl_Box& tilesetresizable = itrnl->Add(new Fl_Box(639, 479, 1, 1));

	tileset_win.end();

	tileset_win.resizable(tilesetresizable);
	tileset_win.set_modal();

	itrnl->tsdt.box_filename = &box_filename;
	itrnl->tsdt.tilesetpreview = &tilesetpreview;
	itrnl->tsdt.tileset_ox = &tileset_ox;
	itrnl->tsdt.tileset_oy = &tileset_oy;
	itrnl->tsdt.tileset_px = &tileset_px;
	itrnl->tsdt.tileset_py = &tileset_py;
	itrnl->tsdt.tileset_tw = &tileset_tw;
	itrnl->tsdt.tileset_th = &tileset_th;
	itrnl->tsdt.tileset_ok = &tileset_ok;


	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& tiles_win = itrnl->Add(new Fl_Window(640, 480, "Tile management"));

	tiles_win.begin();

	MyThumbnailViewer& scroller = itrnl->Add(new MyThumbnailViewer(0, 0, 640, 480));
	scroller.separation(20, 20);
	scroller.begin();

	Fl_Button& tile_add_button = itrnl->Add(new Fl_Button(10, 10, 100, 100, "@+9+"));
	tile_add_button.callback(AddTileButtonCallback, (void*)&itrnl->guidata);

	scroller.end();
	scroller.autoarrange();

	tiles_win.end();
	tiles_win.resizable(scroller);

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& tile_chooser_win = itrnl->Add(new Fl_Window(640, 480, "Choose a tile"));

	tile_chooser_win.begin();

	TileChooser& tile_chooser = itrnl->Add(new TileChooser(&itrnl->guidata));

	tile_chooser_win.end();

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& morceau_level_win = itrnl->Add(new Fl_Window(320, 240, "Brick Heap"));
	morceau_level_win.begin();

	Fl_Menu_Bar& mlvl_bar = itrnl->Add(new Fl_Menu_Bar(0, 0, 320, 30));
	mlvl_bar.add("Mode/Add", 0, 0, 0, FL_MENU_RADIO);
	mlvl_bar.add("Mode/Replace", 0, 0, 0, FL_MENU_RADIO);
	mlvl_bar.add("Copy", 0, 0);
	mlvl_bar.add("Paste", 0, 0);
	mlvl_bar.add("Delete", 0, 0);

	SetMenuItem(mlvl_bar, "Mode/Add");

	Fl_Button& turn_x_l = itrnl->Add(new Fl_Button(10, 50, 30, 30, "@<-"));
	Fl_Button& turn_x_r = itrnl->Add(new Fl_Button(40, 50, 30, 30, "@->"));
	Fl_Button& turn_y_l = itrnl->Add(new Fl_Button(10, 110, 30, 30, "@<-"));
	Fl_Button& turn_y_r = itrnl->Add(new Fl_Button(40, 110, 30, 30, "@->"));
	Fl_Button& turn_z_l = itrnl->Add(new Fl_Button(10, 170, 30, 30, "@<-"));
	Fl_Button& turn_z_r = itrnl->Add(new Fl_Button(40, 170, 30, 30, "@->"));

	turn_x_l.callback(BrickHeapNegRotX, (void*)&itrnl->guidata);
	turn_x_r.callback(BrickHeapPosRotX, (void*)&itrnl->guidata);
	turn_y_l.callback(BrickHeapNegRotY, (void*)&itrnl->guidata);
	turn_y_r.callback(BrickHeapPosRotY, (void*)&itrnl->guidata);
	turn_z_l.callback(BrickHeapNegRotZ, (void*)&itrnl->guidata);
	turn_z_r.callback(BrickHeapPosRotZ, (void*)&itrnl->guidata);

	turn_x_l.labelcolor(FL_RED);
	turn_x_r.labelcolor(FL_RED);
	turn_y_l.labelcolor(FL_GREEN);
	turn_y_r.labelcolor(FL_GREEN);
	turn_z_l.labelcolor(FL_BLUE);
	turn_z_r.labelcolor(FL_BLUE);

	BrickHeapPreview& brickheappreview = itrnl->Add(new BrickHeapPreview(80, 40, 230, 190, &itrnl->guidata));

	morceau_level_win.end();

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& editeur_morceau_lvl = itrnl->Add(new Fl_Window(640, 480, "Brick editor"));
	editeur_morceau_lvl.begin();

	BrickWindow& brickwindow = itrnl->Add(new BrickWindow(320, 145, 300, 300, &itrnl->guidata));

	Fl_Scroll& mlvledit_scroller = itrnl->Add(new Fl_Scroll(0, 0, 640, 116));
	mlvledit_scroller.begin();

	Fl_Button& button_addbrick = itrnl->Add(new Fl_Button(0, 0, 100, 100, "@+9+"));
	button_addbrick.callback(AddBrickCallback, (void*)&itrnl->guidata);

	mlvledit_scroller.end();

	Fl_Box& vrtxlvl_X = itrnl->Add(new Fl_Box(10, 130, 40, 30, "X"));
	Fl_Box& vrtxlvl_Y = itrnl->Add(new Fl_Box(50, 130, 40, 30, "Y"));
	Fl_Box& vrtxlvl_Z = itrnl->Add(new Fl_Box(90, 130, 40, 30, "Z"));
	Fl_Box& vrtxlvl_U = itrnl->Add(new Fl_Box(130, 130, 40, 30, "U"));
	Fl_Box& vrtxlvl_V = itrnl->Add(new Fl_Box(170, 130, 40, 30, "V"));
	Fl_Scroll& vertex_table = itrnl->Add(new Fl_Scroll(10, 160, 216, 160));
	vertex_table.end();

	Fl_Button& add_vertex = itrnl->Add(new Fl_Button(20, 330, 20, 20, "@+"));
	Fl_Button& remove_vertex = itrnl->Add(new Fl_Button(50, 330, 20, 20, "@1+"));
	add_vertex.callback(AddVertexCallback, (void*)&itrnl->guidata);
	remove_vertex.callback(RemoveVertexCallback, (void*)&itrnl->guidata);

	Fl_Scroll& trianglelist = itrnl->Add(new Fl_Scroll(230, 130, 76, 310));
	trianglelist.end();
	trianglelist.color(FL_WHITE);
	Fl_Button& add_triangle = itrnl->Add(new Fl_Button(230, 440, 38, 20, "@+"));
	Fl_Button& remove_triangle = itrnl->Add(new Fl_Button(268, 440, 38, 20, "@1+"));
	add_triangle.callback(AddTriangleCallback, (void*)&itrnl->guidata);
	remove_triangle.callback(RemoveTriangleCallback, (void*)&itrnl->guidata);

	Fl_Button& loadToHeap = itrnl->Add(new Fl_Button(10, 360, 100, 30, "Load to heap"));
	loadToHeap.callback(LoadToHeapCallback, (void*)&itrnl->guidata);

	editeur_morceau_lvl.end();

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Fl_Window& win = itrnl->Add(new Fl_Window(640, 480, "Super mega genial editeur de mes jeux"));

	win.begin();

	Fl_Menu_Bar& menu = itrnl->Add(new Fl_Menu_Bar(0, 0, 640, 30));

	menu.add("Orientation/X", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Orientation/Y", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Orientation/Z", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Etage", 0, 0);
	menu.add("Grille", 0, 0, 0, FL_MENU_TOGGLE);
	menu.add("Camera/Up/X", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Camera/Up/-X", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Camera/Up/Y", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Camera/Up/-Y", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Camera/Up/Z", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Camera/Up/-Z", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Camera/Focus/Actif", 0, 0, 0, FL_MENU_TOGGLE);
	menu.add("Camera/Focus/X", 0, 0, 0, FL_MENU_VALUE);
	menu.add("Camera/Focus/Y", 0, 0, 0, FL_MENU_VALUE);
	menu.add("Camera/Focus/Z", 0, 0, 0, FL_MENU_VALUE);
	menu.add("Edition Objet/Position", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Edition Objet/Rotation", 0, 0, 0, FL_MENU_RADIO);
	menu.add("Edition Objet/Taille", 0, 0, 0, FL_MENU_RADIO);

	SetMenuItem(menu, "Orientation/X");
	SetMenuItem(menu, "Camera/Up/Z");
	SetMenuItem(menu, "Edition Objet/Position");

	int y = 40;
	Fl_Button& tile_courante = itrnl->Add(new Fl_Button(10, y, 50, 50, "Tile"));
	Fl_Button& tilesetmgr_but = itrnl->Add(new Fl_Button(70, y, 50, 50, "TileSet"));

	Fl_Button& morceau_lvl = itrnl->Add(new Fl_Button(10, y += 60, 50, 50, "Brick\nHeap"));
	Fl_Button& morceau_lvl_edit = itrnl->Add(new Fl_Button(70, y, 50, 50, "Bricks"));

	Fl_Button& objet_courant = itrnl->Add(new Fl_Button(10, y += 60, 50, 50, "Obj"));

	Fl_Button& annuler = itrnl->Add(new Fl_Button(10, y += 60, 50, 50, "@undo"));
	Fl_Button& repeter = itrnl->Add(new Fl_Button(70, y, 50, 50, "@redo"));

	Fl_Button& sauvegarder = itrnl->Add(new Fl_Button(10, y += 60, 50, 50, "@filesave"));
	Fl_Button& charger = itrnl->Add(new Fl_Button(70, y, 50, 50, "@fileopen"));

	LevelPreview& levelPreview = itrnl->Add(new LevelPreview(130, 40, 500, 430, &itrnl->guidata));

	objet_courant.callback(OpenWindowCallback, (void*)&object_prop);

#define OPENANDRELOAD(widget, windowtoopen, reloadcallback) MultiArg& multiarg ## widget = *(new MultiArg); \
	itrnl->multiArgs.push_back(&multiarg ## widget); \
	AddInMultiArg(multiarg ## widget, OpenWindowCallback, (void*)&windowtoopen); \
	AddInMultiArg(multiarg ## widget, reloadcallback, (void*)&itrnl->guidata); \
	widget.callback(MultiCallback, (void*)&multiarg ## widget);

	morceau_lvl.callback(OpenWindowCallback, (void*)&morceau_level_win);

	OPENANDRELOAD(tilesetmgr_but, tiles_win, ReloadTilesetDataCallback);
	OPENANDRELOAD(tile_courante, tile_chooser_win, ReloadTilechooserCallback);
	OPENANDRELOAD(morceau_lvl_edit, editeur_morceau_lvl, ReloadBrickEditorCallback);

	annuler.callback(UndoCallback, (void*)&itrnl->guidata);
	repeter.callback(RedoCallback, (void*)&itrnl->guidata);

	sauvegarder.callback(SaveFileCallback, (void*)&itrnl->guidata);
	charger.callback(LoadFileCallback, (void*)&itrnl->guidata);

	win.end();
	//win.show();

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	itrnl->guidata.cur_ts_id = 0;
	itrnl->guidata.cur_tile_x = 0;
	itrnl->guidata.cur_tile_y = 0;
	itrnl->guidata.cur_brick = 0;
	itrnl->guidata.cur_vertex = 0;
	itrnl->guidata.cur_triangle = 0;
	itrnl->guidata.leveldata = &itrnl->leveldata;
	itrnl->guidata.tmdata = &itrnl->tsdt;
	itrnl->guidata.objdata = &itrnl->objdata;
	itrnl->guidata.scroller = &scroller;
	itrnl->guidata.tilechooser = &tile_chooser;
	itrnl->guidata.vertexeditor = &vertex_table;
	itrnl->guidata.triangleeditor = &trianglelist;
	itrnl->guidata.brickchooser = &mlvledit_scroller;
	itrnl->guidata.brickwindow = &brickwindow;
	itrnl->guidata.brickheappreview = &brickheappreview;
	itrnl->guidata.levelPreview = &levelPreview;

	GuiContext* context = new GuiContext;
	context->internal = itrnl;
	context->mainWindow = &win;
	return context;
}

void Gui::Unload(Gui::GuiContext* context) {
	for (int i = (int)context->internal->widgets.size() - 1; i >= 0; --i) {
		delete context->internal->widgets[i];
	}
	for (unsigned int i = 0; i < context->internal->multiArgs.size(); ++i) {
		delete context->internal->multiArgs[i];
	}
	delete context->internal;
	delete context;
}

int Gui::SetupAndLaunch() {
	GuiContext* context = Load();
	int ret = Fl::run();
	Unload(context);
	return ret;
}
