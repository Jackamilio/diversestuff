#ifndef __INSTRUCTION_MODEL_H__
#define __INSTRUCTION_MODEL_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Rect.h"
#include "GuiElement.h"
#include "InstructionContext.h"
#include "bitflag.h"

extern const int paramoffset;
extern const int jumpShift;
extern const int parampadding;

class Instruction;
class InstructionFamily;

class InstructionModel : public GuiElement {
private:
    Instruction* CreateInstruction();

public:
    InstructionFamily& family;
    Rect defaultRect;
    float paramsX;
    const char* text;

    enum class Type { Default, Parameter, Jump };
    Type type;

    enum class Flags { Trigger, Fixed, Visible, StickToPrev, Editable, Number, Multiline };
    bitflag<Flags> flags;

    InstructionModel* prevLink;
    InstructionModel* nextLink;

    InstructionModel* jump;

    int parametersTaken;

    enum class FunctionResult {
        Continue    = 0x00,
        Stop        = 0x01, // ignores all other flags if set
        Jump        = 0x02,
        Await       = 0x04,
        Error       = 0x08,
    };
    std::function<FunctionResult(Parameter*, const Instruction& callinginst, InstructionContext& context)> function;
    std::function<Parameter(Parameter*, const Instruction& callinginst, InstructionContext& context)> evaluate;

    InstructionModel* GetPrevVisibleLink();
    InstructionModel* GetNextVisibleLink();

    InstructionModel(InstructionFamily& fam);
    ~InstructionModel();

    void SetText(const char* t);

    virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);
    void Draw();
    void DrawBack(const Rect& rect) const;
    void DrawText() const;
    void DrawConnexion(const Rect& rect, int connexion = 0) const;
    void DrawAll(const Rect& rect, int connexion = 0) const;

    // assumes both links as Jump type, returns "to" for convenience
    InstructionModel* Link(InstructionModel* to);
    inline void JumpsTo(InstructionModel* to) { jump = to; }
};

// I hate this
inline InstructionModel::FunctionResult operator|(InstructionModel::FunctionResult a, InstructionModel::FunctionResult b)
{
    return static_cast<InstructionModel::FunctionResult>(static_cast<int>(a) | static_cast<int>(b));
}

inline InstructionModel::FunctionResult operator&(InstructionModel::FunctionResult a, InstructionModel::FunctionResult b)
{
    return static_cast<InstructionModel::FunctionResult>(static_cast<int>(a) & static_cast<int>(b));
}

#endif //__INSTRUCTION_MODEL_H__