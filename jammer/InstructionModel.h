#ifndef __INSTRUCTION_MODEL_H__
#define __INSTRUCTION_MODEL_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <functional>
#include "Rect.h"
#include "GuiElement.h"
#include <vector>

extern const int paramoffset;
extern const int jumpShift;

class Instruction;
class InstructionFamily;

typedef float Parameter;
typedef std::vector<Parameter> ParameterList;

class InstructionModel : public GuiElement {
private:
    Instruction* CreateInstruction();

public:
    InstructionFamily& family;
    glm::ivec2 pos;
    Rect defaultRect;
    float paramsX;
    const char* text;

    enum class Type { Default, Parameter, Jump };
    Type type;

    bool isTrigger; // for Default
    bool fixed; // for Parameter

    InstructionModel* jumpAbove;
    InstructionModel* jump;

    int parametersTaken;

    enum class FunctionResult { Continue, Stop, JumpToNext, ElseJump, ElseContinue, Yield, Error };
    std::function<FunctionResult(Parameter*)> function;
    std::function<Parameter(Parameter*)> evaluate;

    InstructionModel(InstructionFamily& fam);
    ~InstructionModel();

    inline void Place(int _x, int _y) {
        pos.x = _x;
        pos.y = _y;
    }

    void SetText(const char* t);

    virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);
    void Draw();

    void Draw(const glm::ivec2& pos, const Rect& rect, int connexion = 0) const;

    // assumes both links as Jump type, returns "to" for convenience
    InstructionModel* Link(InstructionModel* to);
};

#endif //__INSTRUCTION_MODEL_H__