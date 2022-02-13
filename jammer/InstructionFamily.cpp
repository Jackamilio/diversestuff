#include "InstructionFamily.h"
#include <allegro5/allegro_primitives.h>
#include "DefaultColors.h"
#include "InstructionModel.h"
#include <unordered_set>

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
    emptyParameter->flags |= InstructionModel::Flags::Fixed;
    emptyParameter->flags |= InstructionModel::Flags::Editable;
    emptyParameter->flags |= InstructionModel::Flags::Number;
    emptyParameter->SetText("   ");
    emptyParameter->evaluate = [](Parameter*, const Instruction& inst, InstructionContext&) {
        const EditableInstruction& ei = (EditableInstruction&)(inst);
        return atof(al_cstr(ei.GetText()));
    };
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
    context.Clear();
}

std::vector<Parameter> paramMemory;

Parameter* EvaluateParameters(Instruction& inst, InstructionContext& context) {
    if (!inst.parameters.empty()) {
        size_t curmempos = paramMemory.size();
        size_t nextmempos = curmempos + inst.parameters.size();
        paramMemory.resize(nextmempos);
        for (int i = 0; i < (int)inst.parameters.size(); ++i) {
            paramMemory[curmempos+i] = inst.parameters[i]->model.evaluate(EvaluateParameters(*inst.parameters[i], context), *inst.parameters[i], context);
            paramMemory.resize(nextmempos);
        }
        return &paramMemory[curmempos];
    }
    return nullptr;
}

void InstructionFamily::ExecuteFrom(Instruction* inst)
{
    while (inst) {
        InstructionModel::FunctionResult ret = inst->model.function(EvaluateParameters(*inst, context), *inst, context);
        paramMemory.clear();
        if (int(ret & InstructionModel::FunctionResult::Stop)) {
            inst = nullptr;
        }
        else {
            inst = int(ret & InstructionModel::FunctionResult::Jump) ? inst->jump : inst->littleBro;
            if (int(ret & InstructionModel::FunctionResult::Await)) {
                awaitingInstructions[inst] = context.CutContextAtFlaggedDepth();
                inst = nullptr;
            }
        }//Todo : Error
    }
}

void InstructionFamily::Step()
{
    // delayed instruction deletion
    for (auto inst : waitingDestruction) {
        for (Instruction* soondeleted = inst; soondeleted != nullptr; soondeleted = soondeleted->littleBro) {
            auto it = awaitingInstructions.find(soondeleted);
            if (it != awaitingInstructions.end()) {
                it->second.Clear();
                awaitingInstructions.erase(it);
            }
        }
        demoteFromBigBro(inst);
        unorphanParameter(inst);
        delete inst;
    }
    waitingDestruction.clear();

    // execute code!
    context.PushContext(); // first layer of context
    context.FlagCurrentDepth();

    // process awaiting instructions
    std::unordered_set<Instruction*> topbrosfromwait;
    topbrosfromwait.reserve(awaitingInstructions.size()); // little optimisation
    for (auto it = awaitingInstructions.begin(); it != awaitingInstructions.end();) {
        topbrosfromwait.insert(it->first->GetFirstBro());
        context.PushContext(it->second);
        Instruction* from = it->first;
        it = awaitingInstructions.erase(it);
        ExecuteFrom(from);
    }

    for (auto bro : bigBrothers) {
        // don't execute if one of our instruction were actually waiting
        if ((bro->model.flags & InstructionModel::Flags::Trigger) && topbrosfromwait.find(bro) == topbrosfromwait.end()) {
            ExecuteFrom(bro);
        }
    }
    context.PopContext();
    assert(context.IsEmpty() && "Something went wrong, some instruction forgot to pop the context maybe.");
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
