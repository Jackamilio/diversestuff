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
    while (ret && !(ret->flags & Flags::Visible)) {
        ret = ret->prevLink;
    }
    return ret;
}

InstructionModel* InstructionModel::GetNextVisibleLink()
{
    InstructionModel* ret = nextLink;
    while (ret && !(ret->flags & Flags::Visible)) {
        ret = ret->nextLink;
    }
    return ret;
}

InstructionModel::InstructionModel(InstructionFamily& fam) :
    family(fam),
    paramsX(0.0f),
    text(nullptr),
    type(Type::Default),
    flags(Flags::Visible),
    prevLink(nullptr),
    nextLink(nullptr),
    jump(nullptr),
    parametersTaken(0),
    function([](Parameter*, const Instruction&, InstructionContext&) {return FunctionResult::Error; }),
    evaluate([](Parameter*, const Instruction&, InstructionContext&) {return 0.0f; })
{
}

InstructionModel::~InstructionModel()
{
}

void InstructionModel::SetText(const char* t) {
    text = t;
    al_get_text_dimensions(family.font, text, &defaultRect.l, &defaultRect.t, &defaultRect.r, &defaultRect.b);

    defaultRect.expand(type == InstructionModel::Type::Parameter ? parampadding : padding);
    paramsX = defaultRect.right + paramoffset;
    defaultRect.right += (family.emptyParameter->defaultRect.w() + paramoffset) * parametersTaken;
}

Instruction* InstructionModel::CreateInstruction() {
    Instruction* newborn = Instruction::Create(*this);
    newborn->pos = pos;

    glm::ivec2 ppos = pos;
    ppos.x += paramsX;

    for (int i = 0; i < parametersTaken; ++i) {
        newborn->parameters[i] = Instruction::Create(*family.emptyParameter);
        newborn->parameters[i]->pos = ppos;
        newborn->parameters[i]->owner = newborn;
        ppos.x += family.emptyParameter->defaultRect.w() + paramoffset;
    }

    return newborn;
}

bool InstructionModel::Event(ALLEGRO_EVENT& event)
{
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if ((flags & Flags::Visible) && defaultRect.isInside(MousePosition(event))) {
            // create the first instruction that's on top
            InstructionModel* topModel = this;
            while (topModel->prevLink) {
                topModel = topModel->prevLink;
            }
            Instruction* topInst = topModel->CreateInstruction();
            //if (topModel->type == Type::Parameter) {
            //    family.orphanParameter(topInst);
            //}
            //else {
            //    family.promoteToBigBro(topInst);
            //}

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
            return true;
        }
    }
    return false;
}

void InstructionModel::Draw()
{
    if (flags & Flags::Visible) {
        InstructionModel* validPrev = GetPrevVisibleLink();
        DrawAll(defaultRect, validPrev ? pos.y - validPrev->pos.y - defaultRect.h() + 1 : 0);

        //glm::ivec2 ppos = pos;
        //ppos.x += paramsX;

        gui.PushTransform();
        gui.TranslateTransform({ paramsX, 0 });
        for (int i = 0; i < parametersTaken; ++i) {
            family.emptyParameter->DrawAll(family.emptyParameter->defaultRect);
            //ppos.x += family.emptyParameter->defaultRect.w() + paramoffset;
            gui.TranslateTransform({ family.emptyParameter->defaultRect.w() + paramoffset, 0 });
        }
        gui.PopTransform();
    }
}


void InstructionModel::DrawBack(const Rect& rect) const
{
    //Rect rectpos = rect + pos;
    if (type == InstructionModel::Type::Parameter) {
        //rectpos.
            rect.draw_outlined_rounded(6, 6, black, grey, 1);
    }
    else {
        //rectpos.
            rect.draw_outlined(black, grey, 1);
    }
}

void InstructionModel::DrawText() const
{
    al_draw_text(family.font, white, 0, 0, 0, text);
}

void InstructionModel::DrawConnexion(const Rect& rect, int connexion) const
{
    if (connexion) {
        glm::ivec2 tlp = rect.tl;// +pos;
        Rect r(tlp, tlp);

        r.l += 1;
        r.t -= connexion;
        r.r += jumpShift;
        r.b += 1;

        r.draw_filled(black);

        al_draw_line(r.l, r.t, r.l, r.b, grey, 1);
        al_draw_line(r.r, r.t, r.r, r.b, grey, 1);
    }
}

void InstructionModel::DrawAll(const Rect& rect, int connexion) const
{
    DrawBack(rect);
    DrawText();
    DrawConnexion(rect, connexion);
}

InstructionModel* InstructionModel::Link(InstructionModel* to)
{
    assert(type == InstructionModel::Type::Jump && to->type == InstructionModel::Type::Jump && "Only jump types can be linked");
    nextLink = to;
    to->prevLink = this;
    return to;
}
