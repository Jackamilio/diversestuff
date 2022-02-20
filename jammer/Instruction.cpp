#include "InstructionModel.h"
#include "InstructionFamily.h"
#include "DefaultColors.h"
#include <allegro5/allegro_primitives.h>
#include "GuiMaster.h"
#include "Instruction.h"

const int dropL = 20;
const int dropR = 30;

Instruction* Instruction::GetPrevVisibleLink()
{
    Instruction* ret = prevLink;
    while (ret && !(ret->model.flags & InstructionModel::Flags::Visible)) {
        ret = ret->prevLink;
    }
    return ret;
}

Instruction* Instruction::GetNextVisibleLink()
{
    Instruction* ret = nextLink;
    while (ret && !(ret->model.flags & InstructionModel::Flags::Visible)) {
        ret = ret->nextLink;
    }
    return ret;
}

bool Instruction::IsUnderBro(const Instruction& bro, bool checkAdjusted) {
    if (!(bro.model.flags & InstructionModel::Flags::Visible)) return false;
    const glm::ivec2 brobl = (checkAdjusted ? bro.GetAdjustedPos() : bro.pos) + bro.bl();
    const glm::ivec2 mytl = pos + tl;
    return valueInside(mytl.x, brobl.x - dropL, brobl.x + dropR) && valueInside(mytl.y, brobl.y, brobl.y + bro.h());
}

bool Instruction::IsAboveBro(const Instruction& bro, bool checkAdjusted) {
    if (!(bro.model.flags & InstructionModel::Flags::Visible) || (bro.model.flags & InstructionModel::Flags::Trigger)) return false;
    glm::ivec2 bap = checkAdjusted ? bro.GetAdjustedPos() : bro.pos;
    const int broleft = bap.x + bro.tl.x;
    const int broUp = bap.y;
    const glm::ivec2 mybl = pos + bl();
    return valueInside(mybl.x, broleft - dropL, broleft + dropR) && valueInside(mybl.y, broUp - bro.h(), broUp);
}

void RepositionParameters(Instruction* inst, const glm::ivec2& delta) {
    for (auto param : inst->parameters) {
        param->pos += delta;
        RepositionParameters(param, delta);
    }
}

void Instruction::PlaceUnderBigBroRecursive() {
    if (bigBro) {
        glm::ivec2 delta(pos);
        pos = bigBro->pos;
        if (bigBro->nextLink) {
            pos.x += jumpShift;
        }
        if (prevLink) {
            pos.x -= jumpShift;
        }
        pos.x += bigBro->tl.x - tl.x;
        if (bigBro->model.flags & InstructionModel::Flags::Visible) { pos.y += bigBro->h(); }
        delta -= pos;
        RepositionParameters(this, -delta);

        if (littleBro) {
            littleBro->PlaceUnderBigBroRecursive();
        }
    }
}

void Instruction::PlaceAboveLittleBroRecursive() {
    if (littleBro) {
        glm::ivec2 delta(pos);
        pos = littleBro->pos;
        if (littleBro->prevLink) {
            pos.x += jumpShift;
        }
        if (nextLink) {
            pos.x -= jumpShift;
        }
        pos.x += littleBro->tl.x - tl.x;
        if (littleBro->model.flags & InstructionModel::Flags::Visible) { pos.y -= littleBro->h(); }
        delta -= pos;
        RepositionParameters(this, -delta);

        if (bigBro) {
            bigBro->PlaceAboveLittleBroRecursive();
        }
    }
}

void RepositionAllParamsAndResizeOwnersRecursive(Instruction* top, glm::ivec2& posinc) {
    if (top->parameters.empty()) {
        posinc.x += top->w() + paramoffset;
    }
    else {
        float prevx = posinc.x - top->tl.x;
        posinc.x += top->model.paramsX;
        for (auto param : top->parameters) {
            param->pos = posinc;
            RepositionAllParamsAndResizeOwnersRecursive(param, posinc);
        }
        top->br.x = posinc.x - prevx;
        posinc.x += paramoffset;
    }
}

void Instruction::RepositionAllParamsAndResizeOwners()
{
    Instruction* toppestowner = GetToppestOwner();
    glm::ivec2 travpos = toppestowner->pos;
    RepositionAllParamsAndResizeOwnersRecursive(toppestowner, travpos);
}

Instruction* Instruction::GetFirstBro()
{
    Instruction* ret = this;
    while (ret->bigBro) {
        ret = ret->bigBro;
    }
    return ret;
}

Instruction* Instruction::GetLastBro() {
    Instruction* ret = this;
    while (ret->littleBro) {
        ret = ret->littleBro;
    }
    return ret;
}

Instruction* Instruction::GetToppestOwner()
{
    Instruction* toppestowner = nullptr;
    Instruction* nextowner = this;
    while (nextowner) {
        toppestowner = nextowner;
        nextowner = toppestowner->GetOwner();
    }
    return toppestowner;
}

Instruction::Instruction(InstructionModel& model) :
    Rect(model.defaultRect),
    model(model),
    bigBro(nullptr),
    littleBro(nullptr),
    owner(nullptr),
    prevLink(nullptr),
    nextLink(nullptr),
    jump(nullptr)
{
    parameters.reserve((size_t)model.parametersTaken);
    parameters.resize((size_t)model.parametersTaken, nullptr);
}

Instruction::~Instruction()
{
    delete littleBro;
    for (auto param : parameters) {
        delete param;
    }
}

Instruction* Instruction::Create(InstructionModel& model)
{
    return model.flags & InstructionModel::Flags::Editable ? new EditableInstruction(model) : new Instruction(model);
}

void Instruction::DrawHighlight()
{
    // highlighted param
    if (model.family.highlightedParam == this) {
        draw_rounded(6, 6, white, 3);
    }
}

void Instruction::Draw() {
    if (model.flags & InstructionModel::Flags::Visible)
    {
        // shadow
        if (model.family.shadowBro && gui.CurrentDraggable() == this)
        {
            Rect shadowRect = *this + model.family.shadowBroPos - pos;
            shadowRect.draw_filled(lightgrey);
        }

        // self
        Instruction* validPrev = GetPrevVisibleLink();
        model.DrawAll(*this, validPrev ? pos.y - validPrev->pos.y - h() + 1 : 0);

        DrawHighlight();

        // debug
        //if (highlightmyself) {
        //    (*this + pos).draw(green, 3);
        //}
        //if (!currentDropLocation) {
        //    Rect r;
        //    r.tl = pos;
        //    r.resize(6, 6);
        //    r.draw_filled_rounded(2, 2, red);
        //}
    }
}

void ForceAcceptMeAndParams(Instruction* frominst, DropLocation<Instruction>* newdroploc, const glm::ivec2& delta) {
    frominst->pos += delta;
    newdroploc->ForceAccept(frominst);
    for (auto p : frominst->parameters) {
        ForceAcceptMeAndParams(p, newdroploc, delta);
    }
}

void MoveParameters(std::vector<Instruction*>& params, const glm::ivec2& delta) {
    for (auto p : params) {
        if (p) {
            p->pos += delta;
            MoveParameters(p->parameters, delta);
        }
    }
}

bool Instruction::ReplaceParameter(Instruction* oldp, Instruction* newp)
{
    for (int i = 0; i < parameters.size(); ++i) {
        if (oldp == parameters[i]) {
            model.family.unorphanParameter(newp);
            newp->owner = this;
            glm::ivec2 deltapos = oldp->pos - newp->pos;
            ForceAcceptMeAndParams(newp, currentDropLocation, deltapos);
            parameters[i] = newp;

            //glm::ivec2 offset(newp->w() - oldp->w(), 0);
            //for (++i; i < parameters.size(); ++i) {
            //    parameters[i]->pos += offset;
            //    MoveParameters(parameters[i]->parameters, offset);
            //}
            //ResizeOwners(this, RightestPosition(this));

            RepositionAllParamsAndResizeOwners();

            return true;
        }
    }
    return false;
}

void RejectAllParams(Instruction* inst) {
    for (auto param : inst->parameters) {
        if (param->GetDropLocation()) {
            param->GetDropLocation()->Reject(param);
        }
        inst->gui.AddChild(param);
        param->PutOnTop();
        RejectAllParams(param);
    }
}

void Instruction::GrabbedBis() {
    if (prevLink) {
        CancelGrab();
        prevLink->ForceGrab();
        return;
    }
    if (model.flags & InstructionModel::Flags::Fixed) {
        CancelGrab();
        return;
    }

    RejectAllParams(this);

    if (model.type == InstructionModel::Type::Parameter) {
        if (owner) {
            Instruction* newp = Create(*model.family.emptyParameter);
            owner->ReplaceParameter(this, newp);
            owner = nullptr;
            model.family.orphanParameter(this);
        }
    }
    else {
        Instruction* rememberBigBro = bigBro;
        if (bigBro) {
            bigBro->littleBro = nullptr;
            bigBro = nullptr;
            model.family.promoteToBigBro(this);
        }

        Instruction* curBro = this->littleBro;
        std::stack<Instruction*> expectedJumps;
        if (model.type == InstructionModel::Type::Jump) {
            expectedJumps.push(nextLink);
        }
        while (curBro) {
            if (curBro->model.type == InstructionModel::Type::Jump && curBro->prevLink && expectedJumps.empty())
            {
                // break caught bros
                assert(rememberBigBro && "Something went wrong, in this situation at leat one bigbro exist and it is the top jump");
                rememberBigBro->littleBro = curBro;
                curBro->bigBro->littleBro = nullptr;
                curBro->bigBro = rememberBigBro;
                curBro = nullptr;
            }
            else {
                if (curBro->model.type == InstructionModel::Type::Jump) {
                    if (!curBro->prevLink) {
                        expectedJumps.push(curBro->nextLink);
                    }
                    else if (curBro->nextLink) {
                        expectedJumps.top() = curBro->nextLink;
                    }
                    else {
                        expectedJumps.pop();
                    }
                }
                if (curBro->currentDropLocation) {
                    curBro->currentDropLocation->Reject(curBro);
                }
                gui.AddChild(curBro);
                curBro->PutOnTop();
                RejectAllParams(curBro);
                curBro = curBro->littleBro;
            }
        }
    }
}

void ForceAcceptParams(Instruction* inst, DropLocation<Instruction>* newdroploc) {
    for (auto param : inst->parameters) {
        newdroploc->ForceAccept(param);
        ForceAcceptParams(param, newdroploc);
    }
}

void Instruction::DroppedBis() {
    model.family.shadowBro = false;
    // dropped in the model's location means getting rid of self
    if ((GuiElement*)&currentDropLocation->location == model.Parent()) {
        model.family.DestroyInstruction(this);
    }
    else {
        // accept all little bros into the new drop location
        Instruction* curLilBro = this->littleBro;
        while (curLilBro) {
            currentDropLocation->ForceAccept(curLilBro);
            ForceAcceptParams(curLilBro, currentDropLocation);
            curLilBro = curLilBro->littleBro;
        }

        // as well as params
        ForceAcceptParams(this, currentDropLocation);

        if (model.type == InstructionModel::Type::Parameter) {
            Instruction* inst = model.family.highlightedParam;
            if (inst && inst!=this && inst->owner && inst->owner->ReplaceParameter(inst, this)) {
                model.family.DestroyInstruction(inst);
            }
            model.family.highlightedParam = nullptr;
        }
        else {
            PutAtBottom();
            Instruction* lastBro = GetLastBro();
            for (auto bro : model.family) {
                if (bro != this && currentDropLocation == bro->currentDropLocation && IsUnderBro(*bro)) {
                    if (bro->littleBro && (bro->littleBro->model.flags & InstructionModel::Flags::StickToPrev)) {
                        bro->littleBro->pos = bro->pos;
                        bro->littleBro->pos.y += bro->h();
                        bro = bro->littleBro;
                    }
                    bigBro = bro;
                    if (bro->littleBro) {
                        lastBro->littleBro = bro->littleBro;
                        bro->littleBro->bigBro = lastBro;
                    }
                    bro->littleBro = this;

                    model.family.displacedBro = nullptr;
                    PlaceUnderBigBroRecursive();
                    model.family.demoteFromBigBro(this);
                    break;
                }
                else if (bro != lastBro && bro->currentDropLocation == lastBro->currentDropLocation && !bro->bigBro && lastBro->IsAboveBro(*bro)) {
                    lastBro->littleBro = bro;
                    if (bro->bigBro) {
                        bro->bigBro->littleBro = this;
                        this->bigBro = bro->bigBro;
                    }
                    bro->bigBro = lastBro;

                    lastBro->PlaceAboveLittleBroRecursive();
                    model.family.demoteFromBigBro(bro);
                    break;
                }
            }
        }
    }
}

void Instruction::DroppedBack()
{
    if (littleBro) {
        glm::ivec2 savepos = pos;
        pos += currentDropLocation->GetGlobalOffset();
        littleBro->PlaceUnderBigBroRecursive();
        pos = savepos;
    }
}

void Instruction::Dragged(const glm::ivec2& delta) {
    // move the bloc of bros
    Instruction* curBro = this;
    do {
        curBro->pos += delta;
        // the parameters too
        MoveParameters(curBro->parameters, delta);
        curBro = curBro->littleBro;
    } while (curBro);

    InstructionFamily& family = model.family;
    if (model.type == InstructionModel::Type::Parameter) {
        Rect r;
        model.family.highlightedParam = nullptr;
        std::stack<Instruction*> params;
        model.family.BuildParameterStack(params);
        for (; !params.empty(); params.pop()) {
            Instruction* top = params.top();
            if (top != this && top->GetOwner() != this) {
                r = *top;
                r += top->GetAdjustedPos();
                if (r.isInside(tl + pos) || r.isInside(bl() + pos)) {
                    model.family.highlightedParam = params.top();
                    break;
                }
            }
        }
    }
    else {
        // displacement preview before dropping
        if (family.displacedBro && family.displacedBro->bigBro && !IsUnderBro(*family.displacedBro->bigBro, true)) {
            family.displacedBro->PlaceUnderBigBroRecursive();
            family.displacedBro = nullptr;
        }

        if (!family.displacedBro) {
            for (auto bro : family) {
                if (bro != this && bro->littleBro && IsUnderBro(*bro, true)) {
                    family.displacedBro = bro->littleBro;
                    bro->littleBro->pos.y += h();
                    RepositionParameters(bro->littleBro, glm::ivec2(0, h()));
                    if (bro->littleBro->littleBro) {
                        bro->littleBro->littleBro->PlaceUnderBigBroRecursive();
                    }
                }
            }
        }

        // shadow position of the drop
        family.shadowBro = false;
        Instruction* lastBro = GetLastBro();
        for (auto bro : family) {
            if (bro != this && IsUnderBro(*bro, true)) {
                family.shadowBro = true;
                family.shadowBroPos = bro->GetAdjustedPos();
                if (bro->nextLink) {
                    family.shadowBroPos.x += jumpShift;
                }
                family.shadowBroPos.x += bro->tl.x - tl.x;
                family.shadowBroPos.y += bro->br.y - tl.y;
                break;
            }
            else if (bro != lastBro && !bro->bigBro && lastBro->IsAboveBro(*bro, true)) {
                family.shadowBro = true;
                family.shadowBroPos = bro->GetAdjustedPos();
                if (bro->prevLink) {
                    family.shadowBroPos.x += jumpShift;
                }
                family.shadowBroPos.x += bro->tl.x - lastBro->tl.x;
                family.shadowBroPos.y -= lastBro->h();
                break;
            }
        }
    }
}

bool Instruction::hitCheck(const glm::ivec2& opos) const
{
    if (!(model.flags & InstructionModel::Flags::Visible)) return false;
    if (nextLink) {
        Rect r = *this;
        r.resize(jumpShift, nextLink->pos.y - pos.y);
        if (r.isInside(opos)) {
            return true;
        }
    }
    return isInside(opos);
}

glm::ivec2 Instruction::GetAdjustedPos() const
{
    if (currentDropLocation) {
        return currentDropLocation->GetGlobalOffset() + pos;
    }
    else {
        return pos;
    }
}

Engine::InputStatus Instruction::Event(ALLEGRO_EVENT& ev)
{
    if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        //highlightmyself = hitCheck(glm::ivec2(ev.mouse.x, ev.mouse.y));
    }
    
    return Draggable::Event(ev);
}

EditableInstruction::EditableInstruction(InstructionModel& model) : Instruction(model), EditableText("", 0) {
    Init();
    if (model.flags & InstructionModel::Flags::Number) {
        SetNumerical(true, 5);
    }
    canNewLine = (model.flags & InstructionModel::Flags::Multiline);
}

ALLEGRO_FONT* EditableInstruction::Font() const
{
    return model.family.font;
}

const glm::ivec2& EditableInstruction::Pos() const
{
    return pos;
}

void EditableInstruction::MinimalFrame(Rect& inout)
{
    inout = model.defaultRect;
}

void EditableInstruction::Draw()
{
    if (model.flags & InstructionModel::Flags::Visible) {
        model.DrawBack(*this);
        EditableText::DrawText();
        EditableText::DrawCursor();
        DrawHighlight();
    }
}

Engine::InputStatus EditableInstruction::Event(ALLEGRO_EVENT& event)
{
    Engine::InputStatus ret1 = Instruction::Event(event);

    Rect prevrect = *this;
    Engine::InputStatus ret2 = EditableText::Event(event);
    if (prevrect != *this) {
        RepositionAllParamsAndResizeOwners();
    }

    return (ret1 == Engine::InputStatus::grabbed || ret2 == Engine::InputStatus::grabbed) ? Engine::InputStatus::grabbed : Engine::InputStatus::ignored;
}
