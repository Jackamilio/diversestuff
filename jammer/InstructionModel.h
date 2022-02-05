#ifndef __INSTRUCTION_MODEL_H__
#define __INSTRUCTION_MODEL_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <functional>
#include "Rect.h"
#include "GuiElement.h"
#include <vector>

extern const int paramoffset;

class Instruction;
class InstructionFamily;

typedef float Parameter;
typedef std::vector<Parameter> ParameterList;

class InstructionModel : public GuiElement {
public:
    InstructionFamily& family;
    glm::ivec2 pos;
    Rect defaultRect;
    float paramsX;
    const char* text;

    enum class Type { Default, Parameter };
    Type type;

    union {
        bool isTrigger;
        bool fixed;
    };

    int parametersTaken;

    union {
        std::function<bool(Parameter*)> function;
        std::function<Parameter(Parameter*)> evaluate;
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

    void Draw(const glm::ivec2& pos, const Rect& rect) const;
};

#endif //__INSTRUCTION_MODEL_H__