#ifndef __GUI_MASTER_H__
#define __GUI_MASTER_H__

#include <stack>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include "CropperDisplacer.h"
#include "DropLocation.h"

class GuiMaster : virtual public CropperDisplacer, public Engine::Input, public Engine::Graphic {
private:
	using Engine::Input::AddChild;
	using Engine::Input::AddChildBefore;
	using Engine::Input::AddChildAfter;
	using Engine::Input::RemoveChild;
	using Engine::Input::HasChild;
	using Engine::Input::ChildrenSize;
	using Engine::Input::GetChild;

	using Engine::Graphic::AddChild;
	using Engine::Graphic::AddChildBefore;
	using Engine::Graphic::AddChildAfter;
	using Engine::Graphic::RemoveChild;
	using Engine::Graphic::HasChild;
	using Engine::Graphic::ChildrenSize;
	using Engine::Graphic::GetChild;

	std::stack<ALLEGRO_TRANSFORM> transforms;
	void InitTransforms();

	Draggable* trackedDraggable;
	glm::ivec2 draggableGrabbedPosition;
	std::unordered_map<std::type_index, std::vector<DropLocationBase*>> dropLocations;
	DropLocationBase* draggableGrabbedLocation;

public:
	OTN(GuiMaster);

	// BS compiler can't realise they can't use the other overloaded function so I have to manually tell this mf
	inline void AddChild(GuiElement* c, bool onTop = false) { GuiElement::AddChild(c, onTop); }
	inline void AddChildBefore(GuiElement* c, GuiElement* target) { GuiElement::AddChildBefore(c, target); }
	inline void AddChildAfter(GuiElement* c, GuiElement* target) { GuiElement::AddChildAfter(c, target); }
	inline void RemoveChild(GuiElement* c) { GuiElement::RemoveChild(c); }
	inline bool HasChild(GuiElement* c) const { return Arborescent<GuiElement>::HasChild(c); }
	inline int ChildrenSize() const { return Arborescent<GuiElement>::ChildrenSize(); }
	inline GuiElement* GetChild(int i) { return Arborescent<GuiElement>::GetChild(i); }

	static void Init();
	static void End();
	static GuiMaster& Get();

	GuiMaster();
	~GuiMaster();

	Engine::InputStatus Event(ALLEGRO_EVENT& event);
	void Draw();

	virtual glm::ivec2 GetDisplaceOffset() const;

	//
	static bool RecursiveEvent(GuiElement* guielem, ALLEGRO_EVENT& event, bool doroot = true);
	static void RecursiveDraw(GuiElement* guielem, bool doroot = true);

	// drag and drop
	void Track(Draggable* dgbl);
	void UnTrack(Draggable* dgbl);

	bool IsTracked(Draggable* dgbl) const;
	inline bool IsDragging() const { return trackedDraggable != nullptr; }
	inline Draggable* CurrentDraggable() { return trackedDraggable; }
	inline glm::ivec2& CurDraggableGrabbedPosition() { return draggableGrabbedPosition; }
	DropLocationBase*& CurDraggableGrabbedLocation() { return draggableGrabbedLocation; }

	template<class T>
	void AddDropLocation(CropperDisplacer& cpdl);
	template<class T>
	std::vector<DropLocation<T>*>& GetDropLocations();

	// transformations
	void PushTransform();
	void PopTransform();
	void IdentityTransform();
	void TranslateTransform(const glm::ivec2& offset);
	inline ALLEGRO_TRANSFORM& CurrentTransform() { return transforms.top(); }
};

template<class T>
inline void GuiMaster::AddDropLocation(CropperDisplacer& cpdl)
{
	std::type_index id = std::type_index(typeid(T));
	dropLocations[id].push_back(new DropLocation<T>(cpdl));
}

template<class T>
inline std::vector<DropLocation<T>*>& GuiMaster::GetDropLocations()
{
	std::type_index id = std::type_index(typeid(T));
	return *reinterpret_cast<std::vector<DropLocation<T>*>*>(&dropLocations[id]);
}

#endif //__DRAGGABLE_MANAGER_H__