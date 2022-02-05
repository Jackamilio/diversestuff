#include "InstructionFamily.h"
#include <allegro5/allegro_primitives.h>
#include "DefaultColors.h"
#include "InstructionModel.h"

InstructionFamily::Iterator::Iterator(vector<Instruction*>& vec) : bigBroIt(vec.begin()), endBroIt(vec.end()), curBro(*bigBroIt) {}
InstructionFamily::Iterator::Iterator(vector<Instruction*>& vec, int dummy) : bigBroIt(vec.end()), endBroIt(vec.end()), curBro(nullptr) {}

bool InstructionFamily::Iterator::operator !=(const Iterator& r) const {
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
    return bigBrothers.empty() ? end() : Iterator(bigBrothers);
}

InstructionFamily::Iterator InstructionFamily::end() {
    return Iterator(bigBrothers, 0);
}

void BuildParameterStackRec(std::stack<Instruction*>& out, Instruction& inst) {
    for (auto p : inst.parameters) {
        out.push(p);
        BuildParameterStackRec(out, *p);
    }
}

void InstructionFamily::BuildParameterStack(std::stack<Instruction*>& out)
{
    for (auto bro : *this) {
        BuildParameterStackRec(out, *bro);
    }

    for (auto orphan : orphanedParameters) {
        BuildParameterStackRec(out, *orphan);
    }
}

InstructionFamily::InstructionFamily(ALLEGRO_FONT* font) : font(font), shadowBroPos{}
{
    engine.updateRoot.AddChild(this);

    emptyParameter = new InstructionModel(*this);
    emptyParameter->type = InstructionModel::Type::Parameter;
    emptyParameter->fixed = true;
    emptyParameter->SetText("...");
    emptyParameter->evaluate = [](Parameter*) { return 0.0f; };
}

InstructionFamily::~InstructionFamily()
{
    engine.updateRoot.RemoveChild(this);
    for (auto bro : bigBrothers) {
        delete bro;
    }
    for (auto orphan : orphanedParameters) {
        delete orphan;
    }

    delete emptyParameter;
}

std::vector<Parameter> paramMemory;

Parameter* EvaluateParameters(Instruction& inst) {
    if (!inst.parameters.empty()) {
        size_t curmempos = paramMemory.size();
        size_t nextmempos = curmempos + inst.parameters.size();
        paramMemory.resize(nextmempos);
        for (int i = 0; i < (int)inst.parameters.size(); ++i) {
            paramMemory[curmempos+i] = inst.parameters[i]->model.evaluate(EvaluateParameters(*inst.parameters[i]));
            paramMemory.resize(nextmempos);
        }
        return &paramMemory[curmempos];
    }
    return nullptr;
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
    for (auto bro : bigBrothers) {
        if (bro->model.isTrigger) {
            Instruction* curinst = bro;
            while (curinst) {
                if (curinst->model.function(EvaluateParameters(*curinst))) {
                    curinst = curinst->littleBro;
                }
                else {
                    curinst = nullptr;
                }
                paramMemory.clear();
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
