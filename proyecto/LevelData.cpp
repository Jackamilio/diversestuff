#include "LevelData.h"
#include "MathUtils.h"
#include <cfloat>
#include <glm/gtx/norm.hpp>
#include <fstream>

using json = nlohmann::json;

LevelData::LevelData()
{
}

LevelData::~LevelData()
{
	Clear();
}

bool LevelData::Save(const char* filename) {
	json j;

	// TILESETS
	json jtilesets = json::array();
	for (unsigned int i = 0; i < tilesets.size(); ++i) {
		json jts;
		TilesetData* tsd = tilesets[i];
		jts["file"] = tsd->file;
		jts["ox"] = tsd->ox;
		jts["oy"] = tsd->oy;
		jts["px"] = tsd->px;
		jts["py"] = tsd->py;
		jts["tw"] = tsd->tw;
		jts["th"] = tsd->th;

		jtilesets.push_back(jts);
	}
	j["tilesets"] = jtilesets;

	// BRICK DATA
	json jbricks = json::array();
	for (unsigned int i = 0; i < bricks.size(); ++i) {
		json jbd;

		BrickData* bd = bricks[i];

		json jverts = json::array();
		for (int vi = 0; vi < bd->GetVertexCount(); ++vi) {
			const Vertex& v = *bd->GetVertex(vi);
			jverts.push_back({v.x, v.y, v.z, v.u, v.v});
		}
		jbd["vertices"] = jverts;

		const BrickData::TriangleList& tl = bd->GetTriangleList();
		json jtriangles = json::array();
		for (unsigned int ti = 0; ti < tl.size(); ++ti) {
			jtriangles.push_back(tl[ti]);
		}
		jbd["triangles"] = jtriangles;

		jbricks.push_back(jbd);
	}
	j["bricks"] = jbricks;


	// LEVEL BRICK HEAPS
	json jlvl = json::array();
	for (const_iterator it = begin(); it != end(); ++it) {
		const BrickHeap& bh = it->second;
		if (!bh.empty()) {
			json jbh;

			const Coordinate& c = it->first;
			jbh["coord"] = {c.x, c.y, c.z};

			json jheap = json::array();
			for (unsigned int i = 0; i < bh.size(); ++i) {
				const Brick& b = bh[i];
				// lot of different values pushed as int as to make the json not too verbose
				// be wary of the order
				jheap.push_back({
					FindTilesetData(b.tilesetdata),
					b.tilex,
					b.tiley,
					FindBrickData(b.brickdata),
					b.matrix.AsSingleInt()
					});
			}
			jbh["heap"] = jheap;

			jlvl.push_back(jbh);
		}
	}
	j["level"] = jlvl;

	std::ofstream ofs(filename);
	ofs << j << std::endl;

	return true;
}

/*bool LevelData::OldSave(const char * filename)
{
	std::ofstream f;
	f.open(filename, std::ios::out);
	if (f.is_open()) {
		// first tileset and brick data is needed
		f << tilesets.size() << '\n';
		for (unsigned int i = 0; i < tilesets.size(); ++i) {
			tilesets[i]->Save(f);
		}
		f << bricks.size() << '\n';
		for (unsigned int i = 0; i < bricks.size(); ++i) {
			bricks[i]->Save(f);
		}

		// construct a list with non-empty heaps
		std::vector<const Coordinate*> coords;
		std::vector<const BrickHeap*> heaps;
		for (const_iterator it = begin(); it != end(); ++it) {
			if (!it->second.empty()) {
				coords.push_back(&it->first);
				heaps.push_back(&it->second);
			}
		}

		// then finally write the level
		f << heaps.size() << '\n';
		for (unsigned int i = 0; i < heaps.size(); ++i) {
			coords[i]->Save(f);
			f << heaps[i]->size() << '\n';
			for (unsigned int j = 0; j < heaps[i]->size(); ++j) {
				(*heaps[i])[j].Save(f);
				f << '\n';
			}
		}

		// finally the game data descriptor and all the instances
		//f << gameDataDescriptorLocation << '\n';
		//instances.Save(f);

		f.close();
		return true;
	}
	return false;
}*/

bool LevelData::Load(const char* filename) {
	// parse the json
	std::ifstream ifs(filename);
	json j = json::parse(ifs, nullptr, false, false);

	if (j.type() == nlohmann::detail::value_t::discarded) {
		std::cout << "Error parsing the level json. Nothing was loaded" << std::endl;
		return false;
	}

	Clear();

	// TILESETS
	for (auto jts : j["tilesets"]) {
		LevelData::TilesetData* tsd = undoRedoer.UseSPtr(new LevelData::TilesetData());

		tsd->file = jts["file"];
		tsd->ox = jts["ox"];
		tsd->oy = jts["oy"];
		tsd->px = jts["px"];
		tsd->py = jts["py"];
		tsd->tw = jts["tw"];
		tsd->th = jts["th"];

		tilesets.push_back(tsd);
	}

	// BRICKS
	for (auto jbd : j["bricks"]) {
		bricks.push_back(undoRedoer.UseSPtr(new LevelData::BrickData(*this, jbd)));
	}

	// LEVEL BRICK HEAPS
	for (auto lvl : j["level"]) {
		json& jc = lvl["coord"];
		Coordinate c(jc[0], jc[1], jc[2]);

		BrickHeap& ref = level[c];
		for (auto jbh : lvl["heap"]) {
			ref.push_back(Brick(*this, jbh));
		}
	}

	return true;
}

/*bool LevelData::OldLoad(const char* filename)
{
	std::ifstream f;
	f.open(filename, std::ios::in);
	if (f.is_open()) {
		Clear();
		unsigned int nb;
//		std::string str;
//		f >> str;
		f >> nb;
		for (unsigned int i = 0; i < nb; ++i) {
			LevelData::TilesetData* tileset = undoRedoer.UseSPtr(new LevelData::TilesetData());
			tileset->OldLoad(f);
			tilesets.push_back(tileset);
		}
		f >> nb;
		for (unsigned int i = 0; i < nb; ++i) {
			LevelData::BrickData* brick = undoRedoer.UseSPtr(new LevelData::BrickData(*this));
			brick->OldLoad(f);
			bricks.push_back(brick);
		}
		f >> nb;
		for (unsigned int i = 0; i < nb; ++i) {
			Coordinate c;
			c.OldLoad(f);
			unsigned int nb2;
			f >> nb2;
			BrickHeap& ref = level[c];
			for (unsigned int j = 0; j < nb2; ++j) {
				Brick b(*this);
				b.OldLoad(f);
				ref.push_back(b);
			}
		}
		//f >> gameDataDescriptorLocation;
		//GameData::LoadGameDataDescriptor(gameDataDescriptorLocation);
		//instances.Set(f);

		f.close();
		return true;
	}
	return false;
}*/

void LevelData::Clear()
{
	undoRedoer.Clear();
	tilesets.clear();
	bricks.clear();
	level.clear();
	//instances.Clear();
}

bool LevelData::Undo()
{
	return undoRedoer.Undo();
}

bool LevelData::Redo()
{
	return undoRedoer.Redo();
}

#define ReturnFalseOnOutOfRange(vector,index) if (index < 0 || index >= (int)vector.size()) {return false;} 
#define ReturnNullOnOutOfRange(vector,index) if (index < 0 || index >= (int)vector.size()) {return nullptr;} 

const LevelData::TilesetData * LevelData::GetTilesetData(const int index) const
{
	ReturnNullOnOutOfRange(tilesets, index);
	return tilesets[index];
}

int LevelData::FindTilesetData(const TilesetData * tsdt) const
{
	for (unsigned int i = 0; i < tilesets.size(); ++i) {
		if (tsdt == tilesets[i]) {
			return (int)i;
		}
	}
	return -1;
}

bool LevelData::RemoveTilesetData(const int index)
{
	// todo : unvalidate tiles using this tileset
	fprintf(stdout, "Warning, undoing of removing tileset not fully implemented. Expect undefined behaviour.\n");

	return undoRedoer.RemoveThing(tilesets, index, true);
}

void LevelData::AddTilesetData()
{
	undoRedoer.AddThing(tilesets, new TilesetData, true);
}

bool LevelData::UpdateTilesetData(int index, const char * file, unsigned int ox, unsigned int oy, unsigned int px, unsigned int py, unsigned int tw, unsigned int th)
{
	ReturnFalseOnOutOfRange(tilesets, index);
	TilesetData tsdt(file, ox, oy, px, py, tw, th);
	//return UpdateThingInVector(tilesets,index,tsdt);
	undoRedoer.UpdateThing(*tilesets[index], tsdt);
	return true;
}

LevelData::BrickData * LevelData::GetBrickData(const int index)
{
	ReturnNullOnOutOfRange(bricks, index);
	return bricks[index];
}

int LevelData::FindBrickData(const BrickData * bd) const
{
	for (unsigned int i = 0; i < bricks.size(); ++i) {
		if (bd == bricks[i]) {
			return (int)i;
		}
	}
	return -1;
}

bool LevelData::RemoveBrickData(const int index)
{
	// todo : remove all of this brick references in the level
	fprintf(stdout, "Warning, undoing of removing brick data not fully implemented. Expect undefined behaviour.\n");

	return undoRedoer.RemoveThing(bricks, index, true);
}

void LevelData::AddBrickData()
{
	undoRedoer.AddThing(bricks, new BrickData(*this), true);
}

bool PushBrickIfDifferent(LevelData::BrickHeap & heap, const LevelData::Brick & brick) {
	for (unsigned int i = 0; i < heap.size(); ++i) {
		if (heap[i] == brick) {
			return false; // found the same, exit
		}
	}
	heap.push_back(brick);
	return true;
}

void LevelData::StackBrick(const Coordinate & c, const Brick & brick)
{
	BrickHeap& ref = level[c];
	BrickHeap copy = ref;
	if (PushBrickIfDifferent(copy, brick)) {
		undoRedoer.UpdateThing(ref, copy);
	}
}

void LevelData::StackBrick(const Coordinate & c, const BrickHeap & heap)
{
	BrickHeap& ref = level[c];
	BrickHeap copy = ref;
	int nbPushed = 0;
	for (unsigned int i = 0; i < heap.size(); ++i) {
		if (PushBrickIfDifferent(copy, heap[i])) {
			++nbPushed;
		}
	}
	if (nbPushed > 0) {
		undoRedoer.UpdateThing(ref, copy);
	}
}

void LevelData::ClearBrick(const Coordinate & c)
{
	//level[c].clear();
	undoRedoer.UpdateThing(level[c], BrickHeap());
}

void LevelData::UpdateBrickTile(const Coordinate & c, int brickIndex, int tilesetDataIndex, int tx, int ty)
{
	BrickHeap& ref = level[c];
	BrickHeap copy = ref;
	Brick& brick = copy[brickIndex];
	const TilesetData* tsdt = GetTilesetData(tilesetDataIndex);
	if (brick.tilesetdata != tsdt || brick.tilex != tx || brick.tiley != ty) {
		brick.tilesetdata = tsdt;
		brick.tilex = tx;
		brick.tiley = ty;
		undoRedoer.UpdateThing(ref, copy);
	}
}

/*void LevelData::RemoveInstance(const std::string & name)
{
	if (name.find('.') == std::string::npos) {
		undoRedoer.RemoveThing(instances.values, name, true);
	}
}

void LevelData::AddInstance(const std::string& type, const std::string & name)
{
	if (name.find('.') == std::string::npos) {
		undoRedoer.AddThing(instances.values, name, GameData::GenerateContainer(type), true);
	}
}

void LevelData::RenameInstance(const std::string & oldname, const std::string & newname)
{
	if (oldname.find('.') == std::string::npos && newname.find('.') == std::string::npos) {
		undoRedoer.StartGroup();

		GameData::ValueContainer& old = instances.GetField(oldname);
		AddInstance(old.Type(), newname);
		GameData::ValueContainer& neo = instances.GetField(newname);
		undoRedoer.UpdateThing(neo, old);
		RemoveInstance(oldname);

		undoRedoer.EndGroup();
	}
}

void LevelData::UpdateInstance(const std::string & field, const std::string & value)
{
	GameData::ValueContainer& ref = instances.GetField(field);
	GameData::ValueContainer copy = ref;
	std::stringstream ss;
	if (ref.Type() == "string") {
		copy.GetVal<std::string>() = value;
	}
	else {
		copy.Load(ss);
	}
	undoRedoer.UpdateThing(ref, copy);
}*/

bool LevelData::RayCast(const glm::vec3 & rayOrigin, const glm::vec3 & rayDirection, float limit, LevelData::RayCastResult* res) const
{
	glm::vec3 planeX(0.0f), planeY(0.0f), planeZ(0.0f), planeXup(0.0f), planeYup(0.0f), planeZup(0.0f), intsctX(0.0f), intsctY(0.0f), intsct(0.0f), intsctZ(0.0f), cAsVec3(0.0f);
	float distPX, distPY, distPZ, intsctDist, curBrickDist = 0.0f;
	int triangleIndex;

	Coordinate c((int)glm::floor(rayOrigin.x), (int)glm::floor(rayOrigin.y), (int)glm::floor(rayOrigin.z));

	while (curBrickDist <= limit*limit) {
		// check c
		const_iterator brick = level.find(c);
		intsctDist = FLT_MAX;
		if (brick != level.end()) {
			cAsVec3.x = (float)c.x;
			cAsVec3.y = (float)c.y;
			cAsVec3.z = (float)c.z;
			for (unsigned int i = 0; i < brick->second.size(); ++i) {
				if (brick->second[i].RayCast(rayOrigin, rayDirection, cAsVec3, &intsct, &triangleIndex)) {
					if (res) {
						float dist = glm::distance2(rayOrigin, intsct);
						if (dist < intsctDist) {
							intsctDist = dist;
							res->coordinate = c;
							res->brickIndex = i;
							res->intersection = intsct;
							res->triangleIndex = triangleIndex;
						}
					}
					else {
						return true;
					}
				}
			}

			if (intsctDist != FLT_MAX) {
				return true;
			}
		}

		// find next c
		planeXup.x = 1.0f;
		planeYup.y = 1.0f;
		planeZup.z = 1.0f;

		planeX.x = (float)c.x;
		planeY.y = (float)c.y;
		planeZ.z = (float)c.z;

		if (rayDirection.x > 0.0f) { planeX.x += 1.0f; }
		if (rayDirection.y > 0.0f) { planeY.y += 1.0f; }
		if (rayDirection.z > 0.0f) { planeZ.z += 1.0f; }

		distPX = FLT_MAX;
		distPY = FLT_MAX;
		distPZ = FLT_MAX;

		if (linePlaneIntersection(planeX, planeXup, rayOrigin, rayDirection, true, &intsctX)) { distPX = glm::distance2(rayOrigin, intsctX); }
		if (linePlaneIntersection(planeY, planeYup, rayOrigin, rayDirection, true, &intsctY)) { distPY = glm::distance2(rayOrigin, intsctY); }
		if (linePlaneIntersection(planeZ, planeZup, rayOrigin, rayDirection, true, &intsctZ)) { distPZ = glm::distance2(rayOrigin, intsctZ); }

		if (distPX < distPY && distPX < distPZ) {
			curBrickDist = distPX;
			c.x += rayDirection.x > 0.0f ? 1 : -1;
		}
		else if (distPY < distPZ) {
			curBrickDist = distPY;
			c.y += rayDirection.y > 0.0f ? 1 : -1;
		}
		else {
			curBrickDist = distPZ;
			c.z += rayDirection.z > 0.0f ? 1 : -1;
		}
	}

	return false;
}

/*void LevelData::TilesetData::Save(std::ofstream & f) const
{
	//OLD COMMENT f /_OLD COMMENT<< file.size() << ' '_/ << file << ' ';
	f << file << ' ';
	f << ox << ' ' << oy << ' ' << px << ' ' << py << ' ' << tw << ' ' << th << ' ';
	f << '\n';
}*/

/*void LevelData::TilesetData::OldLoad(std::ifstream& f)
{
	//OLD COMMENT
	//unsigned int l;
	//f >> l;
	//char* c = new char[l];
	//f.read(c, l);
	//file = c;
	//delete[] c;
	f >> file >> ox >> oy >> px >> py >> tw >> th;
}*/

LevelData::TilesetData::TilesetData() : file("undefined"), values {0}
{
}

LevelData::TilesetData::TilesetData(const TilesetData & copy)
{
	this->file = copy.file;
	this->ox = copy.ox;
	this->oy = copy.oy;
	this->px = copy.px;
	this->py = copy.py;
	this->th = copy.th;
	this->tw = copy.tw;
}

LevelData::TilesetData::TilesetData(const char * file, int ox, int oy, int px, int py, int tw, int th)
{
	this->file = file;
	this->ox = ox;
	this->oy = oy;
	this->px = px;
	this->py = py;
	this->th = th;
	this->tw = tw;
}

void LevelData::TilesetData::operator=(const TilesetData & copy)
{
	this->file = copy.file;
	this->ox = copy.ox;
	this->oy = copy.oy;
	this->px = copy.px;
	this->py = copy.py;
	this->th = copy.th;
	this->tw = copy.tw;
}

LevelData::TilesetData::~TilesetData()
{
	ox = 0;
}

/*void LevelData::Vertex::Save(std::ofstream & f) const
{
	f << x << ' ' << y << ' ' << z << ' ' << u << ' ' << v << ' ';
	f << '\n';
}

void LevelData::Vertex::OldLoad(std::ifstream & f)
{
	f >> x >> y >> z >> u >> v;
}*/

LevelData::Vertex::Vertex() : x(0.0f), y(0.0f), z(0.0f), u(0.0f), v(0.0f)
{
}

LevelData::Vertex::Vertex(const Vertex & vert) : x(vert.x), y(vert.y), z(vert.z), u(vert.u), v(vert.v)
{
}

LevelData::Vertex::Vertex(const float x, const float y, const float z, const float u, const float v) : x(x), y(y), z(z), u(u), v(v)
{
}

void LevelData::Vertex::operator=(const Vertex & vert)
{
	this->x = vert.x;
	this->y = vert.y;
	this->z = vert.z;
	this->u = vert.u;
	this->v = vert.v;
}

LevelData::Vertex::~Vertex()
{
}

/*void LevelData::BrickData::Save(std::ofstream & f) const
{
	f << vertices.size() << '\n';
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		vertices[i]->Save(f);
	}

	f << triangles.size() << '\n';
	for (unsigned int i = 0; i < triangles.size(); ++i) {
		f << triangles[i] << ' ';
	}
	f << '\n';
}

void LevelData::BrickData::OldLoad(std::ifstream & f)
{
	unsigned int nb;
	f >> nb;
	for (unsigned int i = 0; i < nb; ++i) {
		Vertex* vert = levelData.undoRedoer.UseSPtr(new Vertex());
		vert->OldLoad(f);
		vertices.push_back(vert);
	}
	f >> nb;
	for (unsigned int i = 0; i < nb; ++i) {
		int t;
		f >> t;
		triangles.push_back(t);
	}
}*/

LevelData::BrickData::BrickData(LevelData & ld) : levelData(ld)
{
}

LevelData::BrickData::BrickData(LevelData& ld, nlohmann::json& json) : levelData(ld)
{
	for (auto jv : json["vertices"]) {
		vertices.push_back(levelData.undoRedoer.UseSPtr(new Vertex(jv[0], jv[1], jv[2], jv[3], jv[4])));
	}

	for (auto jt : json["triangles"]) {
		triangles.push_back(jt);
	}
}

const LevelData::Vertex * LevelData::BrickData::GetVertex(const int index) const
{
	ReturnNullOnOutOfRange(vertices, index)
	return vertices[index];
}

bool LevelData::BrickData::RemoveVertex(const int index)
{
	return levelData.undoRedoer.RemoveThing(vertices,index, true);
}

void LevelData::BrickData::AddVertex()
{
	levelData.undoRedoer.AddThing(vertices, new Vertex, true);
}

bool LevelData::BrickData::UpdateVertex(const int index, float x, float y, float z, float u, float v)
{
	ReturnFalseOnOutOfRange(vertices, index);
	Vertex vert(x, y, z, u, v);
	levelData.undoRedoer.UpdateThing(*vertices[index], vert);
	return true;
}

bool LevelData::BrickData::UpdateTriangleList(const TriangleList & tri)
{
	levelData.undoRedoer.UpdateThing(triangles, tri);
	return true;
}

/*void LevelData::Brick::Save(std::ofstream & f) const
{
	f << tilex << ' ' << tiley << ' ' << levelData.FindTilesetData(tilesetdata) << ' ' << levelData.FindBrickData(brickdata) << ' ' << matrix.AsSingleInt() << ' ';
}

void LevelData::Brick::OldLoad(std::ifstream & f)
{
	f >> tilex >> tiley;
	int i;
	f >> i;
	tilesetdata = levelData.GetTilesetData(i);
	f >> i;
	brickdata = levelData.GetBrickData(i);
	f >> i;
	matrix.SetFromSingleInt(i);
}*/

bool LevelData::Brick::operator==(const Brick & comp) const
{
	return brickdata == comp.brickdata && matrix == comp.matrix;
}

bool LevelData::Brick::operator!=(const Brick & comp) const
{
	return brickdata != comp.brickdata || matrix != comp.matrix;
}

LevelData::Brick::Brick(LevelData& ld)
	: levelData(ld)
	, tilex(0)
	, tiley(0)
	, brickdata(0)
	, tilesetdata(0)
{
}

LevelData::Brick::Brick(LevelData& ld, nlohmann::json& json)
	: levelData(ld)
	, tilex(json[1])
	, tiley(json[2])
	, brickdata(nullptr)
	, tilesetdata(nullptr)
{
	tilesetdata = ld.GetTilesetData(json[0]);
	brickdata = ld.GetBrickData(json[3]);
	matrix.SetFromSingleInt(json[4]);
}

LevelData::Brick::Brick(const Brick & b)
	: levelData(b.levelData)
	, tilex(b.tilex)
	, tiley(b.tiley)
	, brickdata(b.brickdata)
	, tilesetdata(b.tilesetdata)
	, matrix(b.matrix)
{
}

void LevelData::Brick::operator=(const Brick & b)
{
	tilex = b.tilex;
	tiley = b.tiley;
	brickdata = b.brickdata;
	tilesetdata = b.tilesetdata;
	matrix = b.matrix;
}

bool LevelData::Brick::RayCast(const glm::vec3 & rayOrigin, const glm::vec3 & rayDirection, const glm::vec3& position, glm::vec3* res, int* triangleIndex) const
{
	glm::mat4 mat;
	glm::vec4 t1, t2, t3, pos(position,0);
	glm::vec3 mRes;
	float dist = FLT_MAX;
	matrix.CalcMatrix(mat);

	const std::vector<int>& tl = brickdata->GetTriangleList();
	for (unsigned int i = 0; i < tl.size(); i += 3) {
		t1 = mat * glm::vec4(brickdata->GetVertex(tl[i])->xyz, 1.0f) + pos;
		t2 = mat * glm::vec4(brickdata->GetVertex(tl[i+1])->xyz, 1.0f) + pos;
		t3 = mat * glm::vec4(brickdata->GetVertex(tl[i+2])->xyz, 1.0f) + pos;
		
		if (lineTriangleIntersection(t1.xyz(), t2.xyz(), t3.xyz(), rayOrigin, rayDirection, true, &mRes)) {
			if (res || triangleIndex) {
				float nDist = glm::distance2(rayOrigin, mRes);
				if (nDist < dist) {
					dist = nDist;
					if (res) { *res = mRes; }
					if (triangleIndex) { *triangleIndex = i / 3; }
				}
			}
			else {
				return true;
			}
		}
	}
	return dist != FLT_MAX;
}

void LevelData::Brick::TransformTriangle(const glm::vec3 & position, int triangleIndex, glm::vec3 & ot1, glm::vec3 & ot2, glm::vec3 & ot3) const
{
	glm::mat4 mat;
	glm::vec4 t1, t2, t3, pos(position, 0);
	matrix.CalcMatrix(mat);
	const std::vector<int>& tl = brickdata->GetTriangleList();
	t1 = mat * glm::vec4(brickdata->GetVertex(tl[triangleIndex*3])->xyz, 1.0f) + pos;
	t2 = mat * glm::vec4(brickdata->GetVertex(tl[triangleIndex*3 + 1])->xyz, 1.0f) + pos;
	t3 = mat * glm::vec4(brickdata->GetVertex(tl[triangleIndex*3 + 2])->xyz, 1.0f) + pos;

	ot1 = t1.xyz();
	ot2 = t2.xyz();
	ot3 = t3.xyz();
}

/*void LevelData::Coordinate::Save(std::ofstream & f) const
{
	f << x << ' ' << y << ' ' << z << ' ';
}

void LevelData::Coordinate::OldLoad(std::ifstream & f)
{
	f >> x >> y >> z;
}*/

LevelData::Coordinate::Coordinate()
	: x(0)
	, y(0)
	, z(0)
{
}

LevelData::Coordinate::Coordinate(int X, int Y, int Z)
	: x(X)
	, z(Z)
	, y(Y)
{
}

LevelData::Coordinate::Coordinate(const Coordinate & c)
	: x(c.x)
	, y(c.y)
	, z(c.z)
{
}
