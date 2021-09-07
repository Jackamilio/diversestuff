#include "Scene.h"

ConstructorCollection::ConstructorCollection() {}

ConstructorCollection::~ConstructorCollection()
{
	for (auto&& it : themap) {
		delete it.second;
	}
}

ConstructorCollection& ConstructorCollection::Get()
{
	static ConstructorCollection theunique;
	return theunique;
}
