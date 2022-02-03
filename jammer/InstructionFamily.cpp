#include "InstructionFamily.h"
#include <allegro5/allegro_primitives.h>
#include "DefaultColors.h"
#include "InstructionModel.h"

InstructionFamily::Iterator::Iterator(vector<Instruction*>& vec) : bigBroIt(vec.begin()), endBroIt(vec.end()), curBro(*bigBroIt) {}
InstructionFamily::Iterator::Iterator(vector<Instruction*>& vec, int dummy) : bigBroIt(vec.end()), endBroIt(vec.end()), curBro(nullptr) {}

bool InstructionFamily::Iterator::operator !=(Iterator& r) const {
    return bigBroIt != r.bigBroIt || curBro != r.curBro;
}

void InstructionFamily::Iterator::operator ++() {
    if (curBro->littleBro) {
        curBro = curBro->littleBro;
    }
    else {
        ++bigBroIt;
        curBro = bigBroIt != endBroIt ? *bigBroIt : nullptr;
    }
}

Instruction* InstructionFamily::Iterator::operator*() {
    return curBro;
}

InstructionFamily::Iterator InstructionFamily::begin() {
    return Iterator(bigBrothers);
}

InstructionFamily::Iterator InstructionFamily::end() {
    return Iterator(bigBrothers, 0);
}

InstructionFamily::InstructionFamily(ALLEGRO_FONT* font) : font(font), shadowBroPos{}
{
    engine.updateRoot.AddChild(this);

    emptyParameter = new InstructionModel(*this);
    emptyParameter->type = InstructionModel::Type::Parameter;
    emptyParameter->SetText("...");
}

InstructionFamily::~InstructionFamily()
{
    engine.updateRoot.RemoveChild(this);
    for (auto bro : bigBrothers) {
        delete bro;
    }

    delete emptyParameter;
}

void InstructionFamily::Step()
{
    // delayed instruction deletion
    for (auto inst : waitingDestruction) {
        demoteFromBigBro(inst);
        unorphanParameter(inst);
        delete inst;
    }
    waitingDestruction.clear();

    // execute code!
    ParameterList dummylist;
    for (auto bro : bigBrothers) {
        if (bro->model.type == InstructionModel::Type::Trigger) {
            Instruction* curinst = bro;
            while (curinst && curinst->model.function(dummylist)) {
                curinst = curinst->littleBro;
            }
        }
    }
}

void InstructionFamily::promoteToBigBro(Instruction* tr) {
    bigBrothers.push_back(tr);
}

void InstructionFamily::demoteFromBigBro(Instruction* tr) {
    auto it = std::find(bigBrothers.begin(), bigBrothers.end(), tr);
    if (it != bigBrothers.end()) {
        bigBrothers.erase(it);
    }
}

void InstructionFamily::orphanParameter(Instruction* tr) {
    orphanedParameters.push_back(tr);
}

void InstructionFamily::unorphanParameter(Instruction* tr) {
    auto it = std::find(orphanedParameters.begin(), orphanedParameters.end(), tr);
    if (it != orphanedParameters.end()) {
        orphanedParameters.erase(it);
    }
}
