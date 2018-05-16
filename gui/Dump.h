#ifndef __DUMP_H__
#define __DUMP_H__

#include <string>
#include <sstream>
//#include <btBulletDynamicsCommon.h>
#include "LevelData.h"

class TextureManager;

//inline glm::vec3 btToVec(const btVector3& bt) {
//	return glm::vec3(bt.x(), bt.y(), bt.z());
//}

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

void DrawGlWireCube(float neg, float pos);
void DrawGlWireCapsule(float radius, float height, int turns = 3);

void DrawBrick(const LevelData::BrickData* brick, float uo = 0.0f, float uf = 1.0f, float vo = 0.0f, float vf = 1.0f);
void DrawBrickHeap(const LevelData::BrickHeap& brickheap, TextureManager& texMngr, bool flipTexV = false);
void DrawLevelData(const LevelData& level, TextureManager& txmgr, bool flipTexV = false);

//btTriangleMesh* ConstructLevelCollision(const LevelData& level);

#endif