#ifndef __GUI_MASTER_H__
#define __GUI_MASTER_H__

#include <stack>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include "GuiElement.h"
#include "DropLocation.h"

class GuiMaster : public virtual GuiElement, public Engine::Input, public Engine::Graphic {
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
	std::unordered_map<std::type_index, std::vector<DropLocationBase*>> dropLocations;
	Draggable::GrabProperties grabbedElementProperties;

	GuiElement* focus;
	ALLEGRO_TIMER* caretTimer;
	bool caretVisible;
	ALLEGRO_USTR* numericalChars;
	std::map<const GuiElement*, ALLEGRO_SYSTEM_MOUSE_CURSOR> requestedCursors;

public:
	OTN(GuiMaster);

	// BS compiler can't realise they can't use the other overloaded function so I have to manually tell this mf
	inline void AddChild(GuiElement* c, GuiElement::Priority p = GuiElement::Priority::Default) { GuiElement::AddChild(c, p); }
	inline void RemoveChild(GuiElement* c) { GuiElement::RemoveChild(c); }

	static void Init();
	static void End();
	static GuiMaster& Get();

	GuiMaster();
	~GuiMaster();

	bool Event(ALLEGRO_EVENT& event);
	void Draw();

	//
	static bool RecursiveEvent(GuiElement* guielem, ALLEGRO_EVENT& event, bool doroot = true);
	void RecursiveDraw(GuiElement* guielem, bool doroot = true);

	// drag and drop
	void Track(Draggable* dgbl);
	void UnTrack(Draggable* dgbl);

	bool IsTracked(Draggable* dgbl) const;
	inline bool IsDragging() const { return trackedDraggable != nullptr; }
	inline Draggable* CurrentDraggable() { return trackedDraggable; }
	inline Draggable::GrabProperties& GrabbedElementProperties() { return grabbedElementProperties; }

	template<class T>
	void AddDropLocation(Cropper& cpdl);
	template<class T>
	std::vector<DropLocation<T>*>& GetDropLocations();

	// focus / text
	void RequestFocus(GuiElement* fe);
	void CancelFocus(GuiElement* fe);
	bool HasFocus(GuiElement* fe) const;
	bool IsCaretVisible() const;
	void ResetCaret();
	inline const ALLEGRO_USTR* GetNumericalChars() const { return numericalChars; }

	// transformations
	void PushTransform();
	void PopTransform();
	void IdentityTransform();
	void TranslateTransform(const glm::ivec2& offset);
	inline ALLEGRO_TRANSFORM& CurrentTransform() { return transforms.top(); }

	// cursor
	void RequestCursor(const GuiElement* requester, ALLEGRO_SYSTEM_MOUSE_CURSOR cursor_id);
	void CancelCursor(const GuiElement* requester);
};

template<class T>
inline void GuiMaster::AddDropLocation(Cropper& cpdl)
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