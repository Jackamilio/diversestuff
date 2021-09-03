#ifndef __LEVEL_EDITOR_H__
#define __LEVEL_EDITOR_H__
#include "Engine.h"
#include "LevelData.h"
#include "Editorcamera.h"
#include "Interpolator.h"

#include "TemporaryOrTesting.h"

class LevelEditor : public Engine::Input, public Engine::Update, public Engine::TripleGraphic {
public:
	Engine& engine;
	EngineLevel& lvl;
	LevelData& lvldt;
	bool showGui;
	bool lastShowGui;
	bool tilemode;
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

	LevelData::BrickHeap brickheap;
	Engine::GraphicTarget brickHeapPreview;

	Engine::GraphicTarget brickDataPreview;
	EditorCamera bdPreviewCamera;

	EditorCamera::DefaultInput levelCamera;
	LevelData::Coordinate gridCenter;
	glm::vec3 gridUp;
	Interpolator<glm::vec3> lerper;
	LevelData::Coordinate lastClic;

	LevelEditor(EngineLevel& lvl);

	bool Event(ALLEGRO_EVENT& event);

	void Draw();
	void SecondDraw();
	void ThirdDraw();

	void Step();

private:
	void HandlePick(bool rightheld, bool middleclick);
	void GlideToNewCenter();
};

#endif //__LEVEL_EDITOR_H__