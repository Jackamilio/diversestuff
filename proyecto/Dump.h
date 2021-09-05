#ifndef __DUMP_H__
#define __DUMP_H__

#include <string>
#include <sstream>
#include <bullet/btBulletDynamicsCommon.h>
#include "MapData.h"

class TextureManager;

inline glm::vec3 btToVec(const btVector3& bt) {
	return glm::vec3(bt.x(), bt.y(), bt.z());
}

template<typename T>
inline std::string valToStr(const T& value)
{
	std::ostringstream ss;
	ss << value;
	return std::string(ss.str());
}

template<typename T>
inline T strToVal(const std::string & str)
{
	T val;

	std::stringstream stream(str);
	stream >> val;
	// note : no fail check
	return val;
}

template<typename T>
inline T strToVal(const char * str)
{
	return(strToVal<T>(std::string(str)));
}

std::string tolower(const std::string& str);
std::string toupper(const std::string& str);

void swapmemorychunks(void* elementA, void* elementB, size_t size);

void eraseAllSubStr(std::string& mainStr, const std::string& toErase);
// make this better : add "../" when the file is outside the working directory
std::string makefilelocal(const std::string& file);

void DrawGlWireCube(float neg, float pos);
void DrawGlWireCapsule(float radius, float height, int turns = 3);

void DrawBrick(const MapData::BrickData* brick, float uo = 0.0f, float uf = 1.0f, float vo = 0.0f, float vf = 1.0f);
void DrawBrick(const MapData::Brick& brick, TextureManager& texMngr, bool flipTexV = false);
void DrawBrickWireframe(const MapData::Brick& brick);
void DrawBrickHeap(const MapData::BrickHeap& brickheap, TextureManager& texMngr, bool flipTexV = false);
void DrawLevelData(const MapData& level, TextureManager& txmgr, bool flipTexV = false);

btTriangleMesh* ConstructLevelCollision(const MapData& level);

#endif