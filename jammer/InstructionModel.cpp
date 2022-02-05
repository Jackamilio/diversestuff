#include "InstructionFamily.h"
#include "InstructionModel.h"
#include "Instruction.h"
#include "DefaultColors.h"

const int padding = 6;
const int parampadding = 3;
const int paramwidth = 20;
const int paramoffset = 5;
const float roundness = 2.0f;

InstructionModel::InstructionModel(InstructionFamily& fam) :
    family(fam),
    pos{},
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
    al_get_text_dimensions(family.font, text, &tl.x, &tl.y, &br.x, &br.y);

    expand(type == InstructionModel::Type::Parameter ? parampadding : padding);
    //br.x += (paramwidth + paramoffset) * parametersTaken;
}

Engine::InputStatus InstructionModel::Event(ALLEGRO_EVENT& event)
{
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (isInside(glm::ivec2(event.mouse.x, event.mouse.y) - pos)) {
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
            ppos.x += w() + paramoffset;

            for (int i = 0; i < parametersTaken; ++i) {
                newborn->parameters[i] = Instruction::Create(*family.emptyParameter);
                newborn->parameters[i]->SetPos(ppos);
                newborn->parameters[i]->owner = newborn;
                ppos.x += family.emptyParameter->w() + paramoffset;
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

    Draw(pos);

    glm::ivec2 ppos = pos;
    ppos.x += w() + paramoffset;

    for (int i = 0; i < parametersTaken; ++i) {
        family.emptyParameter->Draw(ppos);
        ppos.x += family.emptyParameter->w() + paramoffset;
    }
}

void InstructionModel::Draw(const glm::ivec2& pos) const
{
    Rect rect = *this + pos;
    if (type == InstructionModel::Type::Parameter) {
        rect.draw_outlined_rounded(6, 6, black, grey, 1);
    }
    else {
        rect.draw_outlined(black, grey, 1);
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
