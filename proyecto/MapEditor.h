#ifndef __LEVEL_EDITOR_H__
#define __LEVEL_EDITOR_H__
#include "Engine.h"
#include "MapData.h"
#include "Editorcamera.h"
#include "Interpolator.h"

#include "TemporaryOrTesting.h"

class MapEditor : public Engine::Input, public Engine::Update, public Engine::TripleGraphic {
public:
	Engine& engine;
	EngineLevel& lvl;
	MapData& lvldt;
	bool showGui;
	bool lastShowGui;
	bool tilemode;
	int lastTileset;
	int curBrickdata;
	int lastBrickdata;
	int selectedBrickInHeap;
	MapData::TilesetData guiTsd;
	std::vector<MapData::Vertex> guiBdVertices;
	MapData::BrickData::TriangleList guiBdTriangles;
	struct SelectedTile {
		int set = -1, x = 0, y = 0;
	};
	SelectedTile selectedtile{};
	int TSIDtoDel;
	int BDIDtoDel;

	MapData::BrickHeap brickheap;
	Engine::GraphicTarget brickHeapPreview;

	Engine::GraphicTarget brickDataPreview;
	EditorCamera bdPreviewCamera;

	EditorCamera::DefaultInput levelCamera;
	MapData::Coordinate gridCenter;
	glm::vec3 gridUp;
	Interpolator<glm::vec3> lerper;
	MapData::Coordinate lastClic;

	MapEditor(EngineLevel& lvl);

	bool Event(ALLEGRO_EVENT& event);

	void Draw();
	void SecondDraw();
	void ThirdDraw();

	void Step();

private:
	void HandlePick(bool rightheld, bool middleclick);
	void GlideToNewCenter();
	void RotateBrickHeap(void (OrthoMatrix::* rotfunc)());
	void Undo();
	void Redo();
	void ReloadCurrentBrickData();
};

#endif //__LEVEL_EDITOR_H__