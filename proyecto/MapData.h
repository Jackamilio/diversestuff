#ifndef ___LEVEL_DATA_H___
#define ___LEVEL_DATA_H___

#include <string>
#include <vector>
#include <map>
#include "UndoRedoer.h"
#define GLM_FORCE_SWIZZLE
#include <glm/vec3.hpp>
#include <iostream>
#include <fstream>
#include "OrthoMatrix.h"
#include "json.hpp"

class MapData {
public:
	MapData();
	~MapData();

	bool Save(const char* filename);
	bool Load(const char* filename);

	void Clear();

	bool Undo();
	bool Redo();


	// Tileset section
	class TilesetData {
	public:
		TilesetData();
		TilesetData(const TilesetData& copy);
		TilesetData(const char * file, int ox, int oy, int px, int py, int tw, int th);
		void operator=(const TilesetData& copy);
		~TilesetData();
		std::string file;
		union {
			int values[6];
			struct {
				int ox, oy, px, py, tw, th;
			};
		};
	};

	const TilesetData* GetTilesetData(const int index) const;
	int FindTilesetData(const TilesetData* tsdt) const;

	bool RemoveTilesetData(const int index);
	void AddTilesetData();
	bool UpdateTilesetData(int index, const char * file, unsigned int ox, unsigned int oy, unsigned int px, unsigned int py, unsigned int tw, unsigned int th);

	// Brick section
	class Vertex {
	public:
		Vertex();
		Vertex(const Vertex& vert);
		Vertex(const float x, const float y, const float z, const float u, const float v);
		void operator=(const Vertex& vert);
		~Vertex();
		union {
			struct {
				float x, y, z, u, v;
			};
			struct {
				glm::vec3 xyz;
				glm::vec3 uv;
			};
			float values[5];
			
		};
	};

	friend class BrickData;
	class BrickData {
	public:
		BrickData(MapData& ld);
		BrickData(MapData& ld, nlohmann::json& json);
		MapData& levelData;

		typedef std::vector<int> TriangleList;

		const Vertex* GetVertex(const int index) const;
		inline int GetVertexCount() const { return (int)vertices.size(); }
		inline const TriangleList& GetTriangleList() const { return triangles; };

		bool RemoveVertex(const int index);
		void AddVertex();
		bool UpdateVertex(const int index, float x, float y, float z, float u, float v);
		bool UpdateTriangleList(const TriangleList& tri);

	private:
		std::vector<Vertex*> vertices;
		TriangleList triangles;
	};

	BrickData* GetBrickData(const int index);
	int FindBrickData(const BrickData* bd) const;
	inline int GetBrickDataCount() const { return (int)bricks.size(); }

	bool RemoveBrickData(const int index);
	void AddBrickData();

	// Actual level data
	class Coordinate {
	public:
		Coordinate();
		Coordinate(int X, int Y, int Z);
		Coordinate(const Coordinate& c);
		inline void operator=(const Coordinate& c) { x = c.x; y = c.y; z = c.z; }
		inline glm::vec3 asVec3() const { return glm::vec3((float)x, (float)y, (float)z); }

		int x, y, z;

		inline bool operator<(const Coordinate& c) const { return (z != c.z ? z < c.z : (y != c.y ? y < c.y : (x < c.x))); }
		inline bool operator==(const Coordinate& c) const { return x == c.x && y == c.y && z == c.z; }
		inline bool operator!=(const Coordinate& c) const { return x != c.x || y != c.y || z != c.z; }
	};

	class Brick {
	public:
		bool operator==(const Brick& comp) const;
		bool operator!=(const Brick& comp) const;

		Brick(MapData& ld);
		Brick(MapData& ld, nlohmann::json& json);
		Brick(const Brick& b);
		void operator=(const Brick& b);
		MapData& levelData;

		bool RayCast(const glm::vec3 & rayOrigin, const glm::vec3 & rayDirection, const glm::vec3& position, glm::vec3 * res = 0, int* triangleIndex = 0) const;
		void TransformTriangle(const glm::vec3& position, int triangleIndex, glm::vec3& ot1, glm::vec3& ot2, glm::vec3& ot3) const;

		int tilex, tiley;
		const BrickData* brickdata;
		const TilesetData* tilesetdata;
		OrthoMatrix matrix;
	};

	typedef std::vector<Brick> BrickHeap;
	
	void StackBrick(const Coordinate& c, const Brick& brick);
	void StackBrick(const Coordinate& c, const BrickHeap& heap);
	void ClearBrick(const Coordinate& c);
	void UpdateBrickTile(const Coordinate& c, int brickIndex, int tilesetDataIndex, int tx, int ty);
	inline const BrickHeap& GetBrick(const Coordinate& c) { return level[c]; }

	typedef std::map<Coordinate, BrickHeap>::const_iterator const_iterator;
	inline const_iterator begin() const { return level.begin(); }
	inline const_iterator end() const { return level.end(); }

	struct RayCastResult {
		glm::vec3 intersection;
		Coordinate coordinate;
		int brickIndex;
		int triangleIndex;
	};

	bool RayCast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, float limit, RayCastResult* res) const;

private:
	UndoRedoer undoRedoer;

	std::vector<TilesetData*> tilesets;
	std::vector<BrickData*> bricks;
	std::map<Coordinate, BrickHeap> level;
	std::string gameDataDescriptorLocation;
};

#endif//___LEVEL_DATA_H___
