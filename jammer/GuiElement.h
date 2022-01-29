#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

#include "Arborescent.h"
#include "Engine.h"

class GuiMaster;
class CropperDisplacer;

class GuiElement : virtual public Arborescent<GuiElement> {
private:
	GuiElement* parent;

	using Arborescent<GuiElement>::AddChild;
	using Arborescent<GuiElement>::AddChildBefore;
	using Arborescent<GuiElement>::AddChildAfter;
	using Arborescent<GuiElement>::RemoveChild;

	void ReplaceParentFromChild(GuiElement* child);

public:
	GuiMaster& gui;

	inline void AddChild(GuiElement* c, bool onTop = false) {
		ReplaceParentFromChild(c);
		Arborescent<GuiElement>::AddChild(c, onTop);
	}
	inline void AddChildBefore(GuiElement* c, GuiElement* target) {
		ReplaceParentFromChild(c);
		Arborescent<GuiElement>::AddChildBefore(c, target);
	}
	inline void AddChildAfter(GuiElement* c, GuiElement* target) {
		ReplaceParentFromChild(c);
		Arborescent<GuiElement>::AddChildAfter(c, target);
	}
	inline void RemoveChild(GuiElement* c) {
		c->parent = nullptr;
		Arborescent<GuiElement>::RemoveChild(c);
	}

	inline GuiElement* Parent() { return parent; }
	inline const GuiElement* Parent() const { return parent; }

	GuiElement();
	virtual ~GuiElement(); // To be safe

	void PutOnTop();
	void PutAtBottom();

	//int CalculatePriority() const;
	//typedef std::vector<const GuiElement*> Lineage;
	class Lineage : public std::vector<const GuiElement*> {
	public:
		bool operator<(const Lineage& rhs) const;
	};
	Lineage CompileLineage() const;

	virtual Engine::InputStatus Event(ALLEGRO_EVENT& event) { return Engine::InputStatus::ignored; }
	virtual void Draw() {}
	virtual void PostDraw() {}
};

#endif//__GUI_ELEMENT_H__