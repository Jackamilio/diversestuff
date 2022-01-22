#ifndef __GUI_MASTER_H__
#define __GUI_MASTER_H__

#include "Arborescent.h"
#include "Engine.h"
#include <stack>

class GuiMaster;

class GuiElement : virtual public Arborescent<GuiElement> {
public:
	GuiMaster& gui;

	GuiElement();
	virtual ~GuiElement(); // To be safe

	void PutOnTop();

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event) { return Engine::InputStatus::ignored; }
	virtual void Draw() {}
	virtual void PostDraw() {}
};

class Draggable;

class GuiMaster : virtual public GuiElement, public Engine::Input, public Engine::Graphic {
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

public:
	OTN(GuiMaster);

	// BS compiler can't realise they can't use the other overloaded function so I have to manually tell this mf
	inline void AddChild(GuiElement* c, bool onTop = false) { Arborescent<GuiElement>::AddChild(c, onTop); }
	inline void AddChildBefore(GuiElement* c, GuiElement* target) { Arborescent<GuiElement>::AddChildBefore(c, target); }
	inline void AddChildAfter(GuiElement* c, GuiElement* target) { Arborescent<GuiElement>::AddChildAfter(c, target); }
	inline void RemoveChild(GuiElement* c) { Arborescent<GuiElement>::RemoveChild(c); }
	inline bool HasChild(GuiElement* c) const { return Arborescent<GuiElement>::HasChild(c); }
	inline int ChildrenSize() const { return Arborescent<GuiElement>::ChildrenSize(); }
	inline GuiElement* GetChild(int i) { return Arborescent<GuiElement>::GetChild(i); }

	static void Init();
	static void End();
	static GuiMaster& Get();

	GuiMaster();

	Engine::InputStatus Event(ALLEGRO_EVENT& event);
	void Draw();

	//
	static bool RecursiveEvent(GuiElement* guielem, ALLEGRO_EVENT& event, bool doroot = true);
	static void RecursiveDraw(GuiElement* guielem, bool doroot = true);

	// drag and drop
	void Track(Draggable* dgbl);
	void UnTrack(Draggable* dgbl);

	bool IsTracked(Draggable* dgbl) const;
	inline bool IsDragging() const { return trackedDraggable != nullptr; }
	inline Draggable* CurrentDraggable() { return trackedDraggable; }

	// transformations
	void PushTransform();
	void PopTransform();
	void IdentityTransform();
	void TranslateTransform(const glm::ivec2& offset);
	inline ALLEGRO_TRANSFORM& CurrentTransform() { return transforms.top(); }
};

#endif //__DRAGGABLE_MANAGER_H__