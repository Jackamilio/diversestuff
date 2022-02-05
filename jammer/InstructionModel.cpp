#include "InstructionFamily.h"
#include "InstructionModel.h"
#include "Instruction.h"
#include "DefaultColors.h"

const int padding = 6;
const int parampadding = 3;
const int paramoffset = 5;
const float roundness = 2.0f;

InstructionModel::InstructionModel(InstructionFamily& fam) :
    family(fam),
    pos{},
    paramsX(0.0f),
    text(nullptr),
    type(InstructionModel::Type::Default),
    isTrigger(false),
    parametersTaken(0)
{
}

InstructionModel::~InstructionModel()
{
}

void InstructionModel::SetText(const char* t) {
    text = t;
    al_get_text_dimensions(family.font, text, &defaultRect.tl.x, &defaultRect.tl.y, &defaultRect.br.x, &defaultRect.br.y);

    defaultRect.expand(type == InstructionModel::Type::Parameter ? parampadding : padding);
    paramsX = defaultRect.br.x + paramoffset;
    defaultRect.br.x += (family.emptyParameter->defaultRect.w() + paramoffset) * parametersTaken;
}

Engine::InputStatus InstructionModel::Event(ALLEGRO_EVENT& event)
{
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (defaultRect.isInside(glm::ivec2(event.mouse.x, event.mouse.y) - pos)) {
            // create an instruction and force grab it
            Instruction* newborn = Instruction::Create(*this);
            if (type == Type::Parameter) {
                family.orphanParameter(newborn);
            }
            else {
                family.promoteToBigBro(newborn);
            }
            newborn->SetPos(pos);

            glm::ivec2 ppos = pos;
            ppos.x += paramsX;

            for (int i = 0; i < parametersTaken; ++i) {
                newborn->parameters[i] = Instruction::Create(*family.emptyParameter);
                newborn->parameters[i]->SetPos(ppos);
                newborn->parameters[i]->owner = newborn;
                ppos.x += family.emptyParameter->defaultRect.w() + paramoffset;
            }

            newborn->ForceGrab();
            return Engine::InputStatus::grabbed;
        }
    }
    return Engine::InputStatus::ignored;
}

void InstructionModel::Draw()
{
    // faster/cleaner way to do this?
    //static std::vector<Instruction*> defaultparams;
    //defaultparams.resize((size_t)parametersTaken, nullptr);
    //Draw(pos, defaultparams);

    Draw(pos, defaultRect);

    glm::ivec2 ppos = pos;
    ppos.x += paramsX;

    for (int i = 0; i < parametersTaken; ++i) {
        family.emptyParameter->Draw(ppos, family.emptyParameter->defaultRect);
        ppos.x += family.emptyParameter->defaultRect.w() + paramoffset;
    }
}

void InstructionModel::Draw(const glm::ivec2& pos, const Rect& rect) const
{
    Rect rectpos = rect + pos;
    if (type == InstructionModel::Type::Parameter) {
        rectpos.draw_outlined_rounded(6, 6, black, grey, 1);
    }
    else {
        rectpos.draw_outlined(black, grey, 1);
    }
    al_draw_text(family.font, white, pos.x, pos.y, 0, text);

    

    /*
    assert(parametersTaken == params.size());

    rect.shrink(padding - parampadding);
    rect.tl.x = rect.br.x - paramwidth;
    const glm::ivec2 shift(-paramwidth - paramoffset, 0);
    
    for (int i = parametersTaken - 1; i >= 0; --i) {
        if (params[i]) {
            rect += glm::ivec2(-params[i]->model.w() - paramoffset);
        }
        else {
            rect.draw_outlined_rounded(6, 6, white, grey, 0.6f);
            rect += shift;
        }
    }*/
}
