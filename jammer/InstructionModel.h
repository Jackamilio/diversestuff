#ifndef __INSTRUCTION_MODEL_H__
#define __INSTRUCTION_MODEL_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <functional>
#include "Rect.h"
#include "GuiElement.h"
#include <vector>

class Instruction;
class InstructionFamily;

typedef float Parameter;
typedef std::vector<Parameter> ParameterList;

class InstructionModel : public Rect, public GuiElement {
public:
    InstructionFamily& family;
    glm::ivec2 pos;
    const char* text;

    enum class Type { Default, Trigger, Parameter };

    Type type;
    int parametersTaken;

    union {
        std::function<bool(ParameterList&)> function;
        std::function<Parameter(ParameterList&)> evaluate;
    };

    InstructionModel(InstructionFamily& fam);
    ~InstructionModel();

    inline void Place(int _x, int _y) {
        pos.x = _x;
        pos.y = _y;
    }

    void SetText(const char* t);

    virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);
    void Draw();

    void Draw(const glm::ivec2& pos) const;
};

#endif //__INSTRUCTION_MODEL_H__