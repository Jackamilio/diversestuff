#ifndef ___GUI_SETUP_H___
#define ___GUI_SETUP_H___

#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <Fl/Fl_Scroll.H>
#include <Fl/Fl_Select_Browser.h>

#include <string>

#include "LevelData.h"
#include "TextureManager.h"

class TilesetPreview;
class TileChooser;
class MyThumbnailViewer;
class BrickWindow;
class BrickHeapPreview;
class LevelPreview;

struct TilesetManagerData {
	Fl_Box* box_filename;
	Fl_Value_Input* tileset_ox;
	Fl_Value_Input* tileset_oy;
	Fl_Value_Input* tileset_px;
	Fl_Value_Input* tileset_py;
	Fl_Value_Input* tileset_tw;
	Fl_Value_Input* tileset_th;
	TilesetPreview* tilesetpreview;
	Fl_Button* tileset_ok;
};

struct ObjectVariable;

struct ObjectVariableWidgetInfo {
	enum Type { none, tint, tuint, tfloat, tdouble, tstring };
	Type type;
	union {
		const int* vint;
		const unsigned int* vuint;
		const float* vfloat;
		const double* vdouble;
		const std::string* vstring;
	};
	Fl_Widget* widget;
};

struct ObjectVariable {
	LevelData* leveldata;
	std::string field;
	std::string labelstr;
	std::vector<ObjectVariableWidgetInfo> widgets;
};

struct ObjectData {
	std::string lastInstance;
	std::string cur_instance;
	Fl_Input* type_instance;
	Fl_Input* nom_instance;
	Fl_Tabs* tabs;
	Fl_Group* loc_scale;
	Fl_Group* rotation_euler;
	Fl_Group* rotation_quat;
	Fl_Scroll* variables;
	Fl_Select_Browser* browser;
	
	std::list<ObjectVariable> var_content;
};

struct GuiSharedData {
	int cur_ts_id;
	int cur_tile_x;
	int cur_tile_y;

	int cur_brick;
	int cur_vertex;
	int cur_triangle;

	LevelData::BrickHeap brickheap;

	LevelData* leveldata;
	TilesetManagerData* tmdata;
	ObjectData* objdata;

	TextureManager<FltkTexture> tex;

	MyThumbnailViewer* scroller;
	TileChooser* tilechooser;
	BrickWindow* brickwindow;
	BrickHeapPreview* brickheappreview;
	LevelPreview* levelPreview;
	Fl_Scroll* vertexeditor;
	Fl_Scroll* triangleeditor;
	Fl_Scroll* brickchooser;
};

void OpenWindowCallback(Fl_Widget* caller, void* win);
void LoadTilesetImageCallback(Fl_Widget*, void* tilesetmanagerdata);
void LoadFileCallback(Fl_Widget*, void*);
void SaveFileCallback(Fl_Widget*, void*);

void RedrawWidgetCallback(Fl_Widget*, void* widgetToRedraw);
void ReloadLevelPreviewCallback(Fl_Widget*, void* reloaddata);
void ReloadTilesetDataCallback(Fl_Widget*, void* reloaddata);
void ReloadTilechooserCallback(Fl_Widget*, void* reloaddata);
void ReloadVertexEditorCallback(Fl_Widget*, void* reloaddata);
void ReloadTriangleEditorCallback(Fl_Widget*, void* reloaddata);
void ReloadBrickChooserCallback(Fl_Widget*, void* reloaddata);
void ReloadBrickEditorCallback(Fl_Widget*, void* reloaddata);
void ReloadObjNameLocRotScaleCallback(Fl_Widget*, void* reloaddata);
void ReloadObjVariablesCallback(Fl_Widget*, void* reloaddata);
void ReloadObjCallback(Fl_Widget*, void* reloaddata);
void AllReloadCallbacksCallback(Fl_Widget*, void* reloaddata);

void TilesetOkCallback(Fl_Widget* caller, void*tilebutton);
void LoadToHeapCallback(Fl_Widget* caller, void*guidata);

void EditTilesetCallback(Fl_Widget* caller, void* tilebutton);
void AddTileButtonCallback(Fl_Widget* caller, void* reloaddata);
void RemoveTileButtonCallback(Fl_Widget* caller, void* reloaddata);

void EditVertexCallback(Fl_Widget* caller, void* guidata);
void AddVertexCallback(Fl_Widget* caller, void* guidata);
void RemoveVertexCallback(Fl_Widget* caller, void* guidata);

void EditTriangleCallback(Fl_Widget* caller, void* guidata);
void AddTriangleCallback(Fl_Widget* caller, void* guidata);
void RemoveTriangleCallback(Fl_Widget* caller, void* guidata);

void AddBrickCallback(Fl_Widget* caller, void* guidata);
void RemoveBrickCallback(Fl_Widget* caller, void* guidata);
void SetCurrentBrickCallback(Fl_Widget* caller, void* guidata);

void UndoCallback(Fl_Widget* caller, void* reloaddata);
void RedoCallback(Fl_Widget* caller, void* reloaddata);
void MultiCallback(Fl_Widget* caller, void* multiarg);

void BrickHeapNegRotX(Fl_Widget* caller, void* guidata);
void BrickHeapPosRotX(Fl_Widget* caller, void* guidata);
void BrickHeapNegRotY(Fl_Widget* caller, void* guidata);
void BrickHeapPosRotY(Fl_Widget* caller, void* guidata);
void BrickHeapNegRotZ(Fl_Widget* caller, void* guidata);
void BrickHeapPosRotZ(Fl_Widget* caller, void* guidata);

void HighlightVertex(GuiSharedData* gd, int vertid);
void HighlightTriangle(GuiSharedData* gd, int triid);

void EditObjFieldCallback(Fl_Widget * input, void * objvar);
void RenameObjCallback(Fl_Widget * input, void* guidata);
void SelectObjCallback(Fl_Widget * input, void* guidata);

#endif//___GUI_SETUP_H___