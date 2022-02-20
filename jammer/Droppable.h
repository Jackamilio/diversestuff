#ifndef __DROPPABLE_H__
#define __DROPPABLE_H__

#include "Draggable.h"
#include "DropLocation.h"
#include "GuiMaster.h"
#include <map>
#include <utility>

template<class T>
class Droppable : public Draggable {
	friend class DropLocation<T>;
protected:
	DropLocation<T>* currentDropLocation;

public:
	virtual void Grabbed() final;
	virtual void Dropped() final;

	virtual void CancelGrab();

	virtual void GrabbedBis() {};
	virtual void DroppedBis() {};
	virtual void DroppedBack() {};

	Droppable();

	void SetDropLocation(Cropper& cpdl);
	DropLocation<T>* GetDropLocation();
};

template<class T>
inline void Droppable<T>::Grabbed()
{
	if (currentDropLocation) {
		gui.GrabbedElementProperties().location = currentDropLocation;
		currentDropLocation->Reject(this);
		gui.AddChild(this);
		PutOnTop();
	}

	GrabbedBis();
}

template<class T>
inline void Droppable<T>::Dropped()
{
	// try dropping in all drop locations
	glm::ivec2 mousepos(Engine::Input::mouseState.x, Engine::Input::mouseState.y);

	std::vector<DropLocation<T>*> & dropLocations = gui.GetDropLocations<T>();
	std::map<GuiElement::Lineage, std::pair<glm::ivec2, DropLocation<T>*>> foundlocations;

	for (auto loc : dropLocations) {
		// CalculateGlobalOffset is expensive so we store it
		glm::ivec2 globaloffset(loc->location.CalculateGlobalOffset());
		if (loc->location.InsideCropping(mousepos - globaloffset)) {
			foundlocations[loc->location.CompileLineage()] = std::pair<glm::ivec2, DropLocation<T>*>(globaloffset,loc);
		}
	}

	if (!foundlocations.empty()) {
		// this is reimplementing DropLocation<T>::Accept, but CalculateGlobalOffset is not called too much
		std::pair<glm::ivec2, DropLocation<T>*>& firstfound = foundlocations.begin()->second;
		if (currentDropLocation != firstfound.second) {
			pos -= firstfound.first;
			firstfound.second->location.AddChild(this, gui.GrabbedElementProperties().priority);
			currentDropLocation = firstfound.second;
		}
	}
	// if no valid drop location found, warp back to where it was
	else {
		CancelGrab();
		DroppedBack();
	}

	DroppedBis();
}

template<class T>
inline void Droppable<T>::CancelGrab()
{
	Draggable::CancelGrab();
	DropLocationBase* loc = gui.GrabbedElementProperties().location;
	loc = loc ? loc : (DropLocationBase*)gui.GetDropLocations<T>()[0];
	loc->location.AddChild(this);
	pos = gui.GrabbedElementProperties().position;
	currentDropLocation = (DropLocation<T>*)(loc);
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
inline void Droppable<T>::SetDropLocation(Cropper& cpdl)
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

template<class T>
inline DropLocation<T>* Droppable<T>::GetDropLocation()
{
	return currentDropLocation;
}

#endif //__DROPPABLE_H__