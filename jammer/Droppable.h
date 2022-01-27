#ifndef __DROPPABLE_H__
#define __DROPPABLE_H__

#include "Draggable.h"
#include "DropLocation.h"
#include "GuiMaster.h"

template<class T>
class Droppable : public Draggable {
	friend class DropLocation<T>;
protected:
	DropLocation<T>* currentDropLocation;

public:
	virtual void Grabbed() final;
	virtual void Dropped() final;

	virtual void GrabbedBis() {};
	virtual void DroppedBis() {};
	virtual void DroppedBack() {};

	Droppable();

	void SetDropLocation(CropperDisplacer& cpdl);
};

template<class T>
inline void Droppable<T>::Grabbed()
{
	if (currentDropLocation) {
		gui.CurDraggableGrabbedLocation() = currentDropLocation;
		currentDropLocation->Reject(this);
		gui.AddChild(this);
		PutOnTop();
	}

	GrabbedBis();
}

template<class T>
inline void Droppable<T>::Dropped()
{
	bool droplocfound = false;
	glm::ivec2 mousepos(Engine::Input::mouseState.x, Engine::Input::mouseState.y);
	std::vector<DropLocation<T>*>& dropLocations = gui.GetDropLocations<T>();
	for (int i = (int)dropLocations.size() - 1; i >= 0; --i) {
		if (dropLocations[i]->Accept(this, mousepos)) {
			droplocfound = true;
			break;
		}
	}

	// if no valid drop location found, warp back to where it was
	if (!droplocfound) {
		DropLocationBase* loc = gui.CurDraggableGrabbedLocation();
		loc = loc ? loc : (DropLocationBase*)dropLocations[0];
		loc->location.AddChild(this);
		SetPos(gui.CurDraggableGrabbedPosition());
		currentDropLocation = (DropLocation<T>*)(loc);

		DroppedBack();
	}

	DroppedBis();
}

template<class T>
inline Droppable<T>::Droppable() : currentDropLocation(nullptr)
{
	std::vector<DropLocation<T>*>& dl = gui.GetDropLocations<T>();
	assert(!dl.empty() && "Add valid drop locations before creating droppables.");
	currentDropLocation = dl[0];
	currentDropLocation->location.AddChild(this);
}

template<class T>
inline void Droppable<T>::SetDropLocation(CropperDisplacer& cpdl)
{
	for (auto dl : gui.GetDropLocations<T>()) {
		if (&dl->location == &cpdl) {
			currentDropLocation = (DropLocation<T>*)(dl);
			cpdl.AddChild(this);
			return;
		}
	}

	assert(false && "This is not a valid drop location.");
}

#endif //__DROPPABLE_H__