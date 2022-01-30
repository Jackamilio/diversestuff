#include "InstructionModel.h"
#include "InstructionFamily.h"
#include "DefaultColors.h"
#include <allegro5/allegro_primitives.h>
#include "GuiMaster.h"
#include "Instruction.h"

const int dropL = 20;
const int dropR = 30;

bool Instruction::isUnderBro(const Instruction& bro, bool checkAdjusted) {
    const glm::ivec2 brobl = (checkAdjusted ? bro.GetAdjustedPos() : bro.pos) + bro.model.bl();
    const glm::ivec2 mytl = pos + model.tl;
    return valueInside(mytl.x, brobl.x - dropL, brobl.x + dropR) && valueInside(mytl.y, brobl.y, brobl.y + bro.model.h());
}

bool Instruction::isAboveBro(const Instruction& bro, bool checkAdjusted) {
    glm::ivec2 bap = checkAdjusted ? bro.GetAdjustedPos() : bro.pos;
    const int broleft = bap.x + bro.model.tl.x;
    const int broUp = bap.y;
    const glm::ivec2 mybl = pos + model.bl();
    return valueInside(mybl.x, broleft - dropL, broleft + dropR) && valueInside(mybl.y, broUp - bro.model.h(), broUp);
}

void Instruction::placeUnderBigBroRecursive() {
    if (bigBro) {
        pos = bigBro->pos;
        pos.x += bigBro->model.tl.x - model.tl.x;
        pos.y += bigBro->model.h();

        if (littleBro) {
            littleBro->placeUnderBigBroRecursive();
        }
    }
}

void Instruction::placeAboveLittleBroRecursive() {
    if (littleBro) {
        pos = littleBro->pos;
        pos.x += littleBro->model.tl.x - model.tl.x;
        pos.y -= littleBro->model.h();

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
    model(model),
    bigBro(nullptr),
    littleBro(nullptr),
    pos{}
{}

Instruction::~Instruction()
{
    delete littleBro;
}

inline void Instruction::Destroy()
{
    model.family.DestroyInstruction(this);
}

void Instruction::Draw() {
    // shadow
    if (model.family.shadowBro && gui.CurrentDraggable() == this)
    {
        Rect shadowRect = model + model.family.shadowBroPos;
        shadowRect.draw_filled(lightgrey);
    }

    // self
    model.Draw(pos);
}

void Instruction::GrabbedBis() {
    PutOnTop();

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

void Instruction::DroppedBis() {
    // dropped in the model's location means getting rid of self
    if ((GuiElement*)&currentDropLocation->location == model.Parent()) {
        Destroy();
    }
    else {
        // accept all little bros into the new drop location
        Instruction* curLilBro = this->littleBro;
        while (curLilBro) {
            currentDropLocation->ForceAccept(curLilBro);
            curLilBro = curLilBro->littleBro;
        }

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

        PutAtBottom();
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

void Instruction::Dragged(const glm::ivec2& delta) {
    // move the bloc of bros
    Instruction* curBro = this;
    do {
        curBro->pos += delta;
        curBro = curBro->littleBro;
    } while (curBro);

    InstructionFamily& family = model.family;

    // displacement preview before dropping
    if (family.displacedBro && family.displacedBro->bigBro && !isUnderBro(*family.displacedBro->bigBro, true)) {
        family.displacedBro->placeUnderBigBroRecursive();
        family.displacedBro = nullptr;
    }

    if (!family.displacedBro) {
        for (auto bro : family) {
            if (bro != this && bro->littleBro && isUnderBro(*bro, true)) {
                family.displacedBro = bro->littleBro;
                bro->littleBro->pos.y += model.br.y - model.tl.y;
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
            family.shadowBroPos.x += bro->model.tl.x - model.tl.x;
            family.shadowBroPos.y += bro->model.br.y - model.tl.y;
            break;
        }
        else if (bro != lastBro && !bro->bigBro && lastBro->isAboveBro(*bro, true)) {
            family.shadowBro = true;
            family.shadowBroPos = bro->GetAdjustedPos();
            family.shadowBroPos.x += bro->model.tl.x - lastBro->model.tl.x;
            family.shadowBroPos.y -= lastBro->model.h();
            break;
        }
    }
}

bool Instruction::hitCheck(const glm::ivec2& opos) const
{
    return model.isInside(opos - pos);
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
