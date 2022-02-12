#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Droppable.h"
#include "EditableText.h"

class InstructionModel;

class Instruction : virtual public Rect, virtual public Droppable<Instruction> {
    friend class InstructionFamily;
    friend class InstructionModel;
protected:
    Instruction* bigBro;
    Instruction* littleBro;

    Instruction* owner; // for parameter

    Instruction* prevLink;
    Instruction* nextLink;

    Instruction* jump;

    Instruction* GetPrevVisibleLink();
    Instruction* GetNextVisibleLink();

    bool IsUnderBro(const Instruction& bro, bool checkAdjusted = false);
    bool IsAboveBro(const Instruction& bro, bool checkAdjusted = false);

    void PlaceUnderBigBroRecursive();
    void PlaceAboveLittleBroRecursive();
    void RepositionAllParamsAndResizeOwners();

    Instruction* GetFirstBro();
    Instruction* GetLastBro();
    Instruction* GetToppestOwner();

    void DrawHighlight();

    Instruction(InstructionModel& model);
    virtual ~Instruction();
public:
    glm::ivec2 pos;
    InstructionModel& model;
    std::vector<Instruction*> parameters;

    static Instruction* Create(InstructionModel& model);

    void Draw();

    bool ReplaceParameter(Instruction* oldp, Instruction* newp);

    virtual void GrabbedBis();
    virtual void DroppedBis();
    virtual void DroppedBack();
    virtual void Dragged(const glm::ivec2& delta);

    virtual bool hitCheck(const glm::ivec2& pos) const;
    virtual void SetPos(const glm::ivec2& tsl);
    virtual glm::ivec2 GetPos() const;

    glm::ivec2 GetAdjustedPos() const;

    inline Instruction* GetOwner() const { return owner; }

    // for debug
    bool highlightmyself = false;
    virtual Engine::InputStatus Event(ALLEGRO_EVENT& ev);
};

class EditableInstruction : public Instruction, public EditableText {
    friend class Instruction;
private:
    EditableInstruction(InstructionModel& model);

public:

    virtual ALLEGRO_FONT* Font() const;
    virtual const glm::ivec2& Pos() const;
    virtual void MinimalFrame(Rect& inout);

    void Draw();
    Engine::InputStatus Event(ALLEGRO_EVENT& event);
};

#endif //__INSTRUCTION_H__