#ifndef __DRAGGABLE_MANAGER_H__
#define __DRAGGABLE_MANAGER_H__

class Draggable;

class DraggableManager {
protected:
	Draggable* trackedDraggable;

public:
	DraggableManager();

	void Track(Draggable* dgbl);
	void UnTrack(Draggable* dgbl);

	bool IsTracked(Draggable* dgbl) const;
	inline bool IsDragging() const { return trackedDraggable != nullptr; }
};

#endif //__DRAGGABLE_MANAGER_H__