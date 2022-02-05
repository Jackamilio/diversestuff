#include "InstructionModel.h"
#include "InstructionFamily.h"
#include "DefaultColors.h"
#include <allegro5/allegro_primitives.h>
#include "GuiMaster.h"
#include "Instruction.h"

const int dropL = 20;
const int dropR = 30;

bool Instruction::isUnderBro(const Instruction& bro, bool checkAdjusted) {
    const glm::ivec2 brobl = (checkAdjusted ? bro.GetAdjustedPos() : bro.pos) + bro.bl();
    const glm::ivec2 mytl = pos + tl;
    return valueInside(mytl.x, brobl.x - dropL, brobl.x + dropR) && valueInside(mytl.y, brobl.y, brobl.y + bro.h());
}

bool Instruction::isAboveBro(const Instruction& bro, bool checkAdjusted) {
    glm::ivec2 bap = checkAdjusted ? bro.GetAdjustedPos() : bro.pos;
    const int broleft = bap.x + bro.tl.x;
    const int broUp = bap.y;
    const glm::ivec2 mybl = pos + bl();
    return valueInside(mybl.x, broleft - dropL, broleft + dropR) && valueInside(mybl.y, broUp - bro.h(), broUp);
}

void Instruction::placeUnderBigBroRecursive() {
    if (bigBro) {
        pos = bigBro->pos;
        pos.x += bigBro->tl.x - tl.x;
        pos.y += bigBro->h();

        if (littleBro) {
            littleBro->placeUnderBigBroRecursive();
        }
    }
}

void Instruction::placeAboveLittleBroRecursive() {
    if (littleBro) {
        pos = littleBro->pos;
        pos.x += littleBro->tl.x - tl.x;
        pos.y -= littleBro->h();

        if (bigBro) {
            bigBro->placeAboveLittleBroRecursive();
        }
    }
}

Instruction* Instruction::getLastBro() {
    Instruction* ret = this;
    while (ret->littleBro) {
        ret = ret->littleBro;
    }
    return ret;
}

Instruction::Instruction(InstructionModel& model) :
    Rect(model.defaultRect),
    model(model),
    bigBro(nullptr),
    littleBro(nullptr),
    pos{}
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

void Instruction::Draw() {
    // shadow
    if (model.family.shadowBro && gui.CurrentDraggable() == this)
    {
        Rect shadowRect = *this + model.family.shadowBroPos;
        shadowRect.draw_filled(lightgrey);
    }

    // self
    model.Draw(pos, *this);

    // highlighted param
    if (model.family.highlightedParam == this) {
        (*this + pos).draw_rounded(6, 6, white, 3);
    }
}

void ForceAcceptMeAndParams(Instruction* frominst, DropLocation<Instruction>* newdroploc, const glm::ivec2& delta) {
    frominst->pos += delta;
    newdroploc->ForceAccept(frominst);
    for (auto p : frominst->parameters) {
        ForceAcceptMeAndParams(p, newdroploc, delta);
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
            glm::ivec2 offset(newp->w() - oldp->w(), 0);
            parameters[i] = newp;
            for (++i; i < parameters.size(); ++i) {
                parameters[i]->pos += offset;
            }
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
    if (model.type == InstructionModel::Type::Parameter && model.fixed) {
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
        if (bigBro) {
            bigBro->littleBro = nullptr;
            bigBro = nullptr;
            model.family.promoteToBigBro(this);
        }

        Instruction* curBro = this->littleBro;
        while (curBro) {
            if (curBro->currentDropLocation) {
                curBro->currentDropLocation->Reject(curBro);
            }
            gui.AddChild(curBro);
            curBro->PutOnTop();
            curBro = curBro->littleBro;
        }
    }
}

void Instruction::DroppedBis() {
    // dropped in the model's location means getting rid of self
    if ((GuiElement*)&currentDropLocation->location == model.Parent()) {
        model.family.DestroyInstruction(this);
    }
    else {
        // accept all little bros into the new drop location
        Instruction* curLilBro = this->littleBro;
        while (curLilBro) {
            currentDropLocation->ForceAccept(curLilBro);
            curLilBro = curLilBro->littleBro;
        }

        // as well as params
        for (auto param : parameters) {
            currentDropLocation->ForceAccept(param);
        }

        if (model.type == InstructionModel::Type::Parameter) {
            Instruction* inst = model.family.highlightedParam;
            if (inst && inst->owner && inst->owner->ReplaceParameter(inst, this)) {
                model.family.DestroyInstruction(inst);
            }
            model.family.highlightedParam = nullptr;
        }
        else {
            PutAtBottom();
            Instruction* lastBro = getLastBro();
            for (auto bro : model.family) {
                if (bro != this && currentDropLocation == bro->currentDropLocation && isUnderBro(*bro)) {
                    bigBro = bro;
                    if (bro->littleBro) {
                        lastBro->littleBro = bro->littleBro;
                        bro->littleBro->bigBro = lastBro;
                    }
                    bro->littleBro = this;

                    model.family.displacedBro = nullptr;
                    placeUnderBigBroRecursive();
                    model.family.demoteFromBigBro(this);
                    break;
                }
                else if (bro != lastBro && bro->currentDropLocation == lastBro->currentDropLocation && !bro->bigBro && lastBro->isAboveBro(*bro)) {
                    lastBro->littleBro = bro;
                    if (bro->bigBro) {
                        bro->bigBro->littleBro = this;
                        this->bigBro = bro->bigBro;
                    }
                    bro->bigBro = lastBro;

                    lastBro->placeAboveLittleBroRecursive();
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
        littleBro->placeUnderBigBroRecursive();
        pos = savepos;
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
            if (params.top() != this) {
                r = *params.top();
                r += params.top()->GetAdjustedPos();
                if (r.isInside(tl + pos) || r.isInside(bl() + pos)) {
                    model.family.highlightedParam = params.top();
                    break;
                }
            }
        }
    }
    else {
        // displacement preview before dropping
        if (family.displacedBro && family.displacedBro->bigBro && !isUnderBro(*family.displacedBro->bigBro, true)) {
            family.displacedBro->placeUnderBigBroRecursive();
            family.displacedBro = nullptr;
        }

        if (!family.displacedBro) {
            for (auto bro : family) {
                if (bro != this && bro->littleBro && isUnderBro(*bro, true)) {
                    family.displacedBro = bro->littleBro;
                    bro->littleBro->pos.y += h();
                    if (bro->littleBro->littleBro) {
                        bro->littleBro->littleBro->placeUnderBigBroRecursive();
                    }
                }
            }
        }

        // shadow position of the drop
        family.shadowBro = false;
        Instruction* lastBro = getLastBro();
        for (auto bro : family) {
            if (bro != this && isUnderBro(*bro, true)) {
                family.shadowBro = true;
                family.shadowBroPos = bro->GetAdjustedPos();
                family.shadowBroPos.x += bro->tl.x - tl.x;
                family.shadowBroPos.y += bro->br.y - tl.y;
                break;
            }
            else if (bro != lastBro && !bro->bigBro && lastBro->isAboveBro(*bro, true)) {
                family.shadowBro = true;
                family.shadowBroPos = bro->GetAdjustedPos();
                family.shadowBroPos.x += bro->tl.x - lastBro->tl.x;
                family.shadowBroPos.y -= lastBro->h();
                break;
            }
        }
    }
}

bool Instruction::hitCheck(const glm::ivec2& opos) const
{
    return isInside(opos - pos);
}

void Instruction::SetPos(const glm::ivec2& tsl)
{
    pos = tsl;
}

glm::ivec2 Instruction::GetPos() const
{
    return pos;
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
