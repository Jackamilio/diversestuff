#include "DraggableManager.h"
#include "Draggable.h"

DraggableManager::DraggableManager() : trackedDraggable(nullptr) {}

void DraggableManager::Track(Draggable* dgbl)
{
	if (trackedDraggable && trackedDraggable != dgbl) {
		trackedDraggable->Dropped();
	}

	trackedDraggable = dgbl;
}

void DraggableManager::UnTrack(Draggable* dgbl)
{
	if (trackedDraggable == dgbl) {
		trackedDraggable = nullptr;
	}
}

bool DraggableManager::IsTracked(Draggable* dgbl) const
{
	return trackedDraggable == dgbl;
}
