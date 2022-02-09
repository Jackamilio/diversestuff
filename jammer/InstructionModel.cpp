#include "InstructionFamily.h"
#include "InstructionModel.h"
#include "Instruction.h"
#include "DefaultColors.h"

const int padding = 6;
const int parampadding = 3;
const int paramoffset = 5;
const float roundness = 2.0f;
const int jumpShift = 16;

InstructionModel* InstructionModel::GetPrevVisibleLink()
{
    InstructionModel* ret = prevLink;
    while (ret && !ret->visible) {
        ret = ret->prevLink;
    }
    return ret;
}

InstructionModel* InstructionModel::GetNextVisibleLink()
{
    InstructionModel* ret = nextLink;
    while (ret && !ret->visible) {
        ret = ret->nextLink;
    }
    return ret;
}

InstructionModel::InstructionModel(InstructionFamily& fam) :
    family(fam),
    pos{},
    paramsX(0.0f),
    text(nullptr),
    type(InstructionModel::Type::Default),
    isTrigger(false),
    fixed(false),
    visible(true),
    stickToPrev(false),
    prevLink(nullptr),
    nextLink(nullptr),
    jump(nullptr),
    parametersTaken(0),
    function([](Parameter*, InstructionContext&) {return FunctionResult::Error; }),
    evaluate([](Parameter*, InstructionContext&) {return 0.0f; })
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

Instruction* InstructionModel::CreateInstruction() {
    Instruction* newborn = Instruction::Create(*this);
    newborn->SetPos(pos);

    glm::ivec2 ppos = pos;
    ppos.x += paramsX;

    for (int i = 0; i < parametersTaken; ++i) {
        newborn->parameters[i] = Instruction::Create(*family.emptyParameter);
        newborn->parameters[i]->SetPos(ppos);
        newborn->parameters[i]->owner = newborn;
        ppos.x += family.emptyParameter->defaultRect.w() + paramoffset;
    }

    return newborn;
}

Engine::InputStatus InstructionModel::Event(ALLEGRO_EVENT& event)
{
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (visible && defaultRect.isInside(glm::ivec2(event.mouse.x, event.mouse.y) - pos)) {
            // create the first instruction that's on top
            InstructionModel* topModel = this;
            while (topModel->prevLink) {
                topModel = topModel->prevLink;
            }
            Instruction* topInst = topModel->CreateInstruction();
            if (topModel->type == Type::Parameter) {
                family.orphanParameter(topInst);
            }
            else {
                family.promoteToBigBro(topInst);
            }

            // create subsquent instructions that are linked to the first
            std::map<InstructionModel*, Instruction*> couples;
            couples[topModel] = topInst;

            Instruction* prev = topInst;
            InstructionModel* nextLink = topModel->nextLink;
            while (nextLink) {
                Instruction* next = nextLink->CreateInstruction();
                prev->littleBro = next;
                next->bigBro = prev;
                prev->nextLink = next;
                next->prevLink = prev;
                prev = next;
                couples[nextLink] = next;
                nextLink = nextLink->nextLink;
            }

            // resolve jumps
            for (auto couple : couples) {
                if (couple.first->jump) {
                    couple.second->jump = couples[couple.first->jump];
                }
            }

            // grab the first new instance that should repercutate on all next
            topInst->ForceGrab();
            return Engine::InputStatus::grabbed;
        }
    }
    return Engine::InputStatus::ignored;
}

void InstructionModel::Draw()
{
    if (visible) {
        InstructionModel* validPrev = GetPrevVisibleLink();
        Draw(pos, defaultRect, validPrev ? pos.y - validPrev->pos.y - defaultRect.h() + 1 : 0);

        glm::ivec2 ppos = pos;
        ppos.x += paramsX;

        for (int i = 0; i < parametersTaken; ++i) {
            family.emptyParameter->Draw(ppos, family.emptyParameter->defaultRect);
            ppos.x += family.emptyParameter->defaultRect.w() + paramoffset;
        }
    }
}

void InstructionModel::Draw(const glm::ivec2& pos, const Rect& rect, int connexion) const
{
    Rect rectpos = rect + pos;
    if (type == InstructionModel::Type::Parameter) {
        rectpos.draw_outlined_rounded(6, 6, black, grey, 1);
    }
    else {
        rectpos.draw_outlined(black, grey, 1);
    }
    al_draw_text(family.font, white, pos.x, pos.y, 0, text);

    if (connexion) {
        Rect r;
        
        r.tl.x = rectpos.tl.x + 1;
        r.tl.y = rectpos.tl.y - connexion;
        r.br.x = rectpos.tl.x + jumpShift;
        r.br.y = rectpos.tl.y + 1;
        
        r.draw_filled(black);

        glm::ivec2 tr = r.tr();
        glm::ivec2 bl = r.bl();

        al_draw_line(r.tl.x, r.tl.y, bl.x, bl.y, grey, 1);
        al_draw_line(tr.x, tr.y, r.br.x, r.br.y, grey, 1);
    }

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

InstructionModel* InstructionModel::Link(InstructionModel* to)
{
    assert(type == InstructionModel::Type::Jump && to->type == InstructionModel::Type::Jump && "Only jump types can be linked");
    nextLink = to;
    to->prevLink = this;
    return to;
}
