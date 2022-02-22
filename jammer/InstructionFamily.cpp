#include "InstructionFamily.h"
#include <allegro5/allegro_primitives.h>
#include "DefaultColors.h"
#include "InstructionModel.h"
#include <unordered_set>

CodeSpace::~CodeSpace()
{
    for (auto bro : bigBrothers) {
        delete bro;
    }
}

InstructionFamily::Iterator::Iterator(CodeSpaceSet& css) :
    spaceIt(css.begin()),
    endSpaceIt(css.end()),
    bigBroIt((*spaceIt)->bigBrothers.begin()),
    curBro(*bigBroIt)
{}
InstructionFamily::Iterator::Iterator(CodeSpaceSet& css, int dummy) :
    spaceIt(css.end()),
    endSpaceIt(css.end()),
    curBro(nullptr) {}

bool InstructionFamily::Iterator::operator !=(const Iterator& r) const {
    return curBro != r.curBro; //in theory there is no duplicate
}

void InstructionFamily::Iterator::operator ++() {
    if (curBro->littleBro) {
        curBro = curBro->littleBro;
    }
    else {
        ++bigBroIt;
        if (bigBroIt == (*spaceIt)->bigBrothers.end()) {
            ++spaceIt;
            if (spaceIt == endSpaceIt) {
                curBro = nullptr;
                return;
            }
        }
        curBro = *bigBroIt;
    }
}

Instruction* InstructionFamily::Iterator::operator*() {
    return curBro;
}

InstructionFamily::Iterator InstructionFamily::begin() {
    //return bigBrothers.empty() ? end() : Iterator(bigBrothers);
    return !codeSpaces.empty() && !(*codeSpaces.begin())->bigBrothers.empty() ? Iterator(codeSpaces) : end();
}

InstructionFamily::Iterator InstructionFamily::end() {
    //return Iterator(bigBrothers, 0);
    return Iterator(codeSpaces, 0);
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
    //for (auto bro : bigBrothers) {
    //    delete bro;
    //}
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

void InstructionFamily::ExecuteFrom(Instruction* inst, CodeInstance& code)
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
                code.awaitingInstructions[inst] = context.CutContextAtFlaggedDepth();
                inst = nullptr;
            }
        }//Todo : Error
    }
}

void InstructionFamily::ExecuteCode(CodeInstance& code)
{
    std::unordered_map<Instruction*, InstructionContext>& waiting = code.awaitingInstructions;
    // delayed instruction deletion
    for (auto inst : waitingDestruction) {
        for (Instruction* soondeleted = inst; soondeleted != nullptr; soondeleted = soondeleted->littleBro) {
            auto it = waiting.find(soondeleted);
            if (it != waiting.end()) {
                it->second.Clear();
                waiting.erase(it);
            }
        }
        //demoteFromBigBro(inst);
        //unorphanParameter(inst);
        
        //not deleting here now, but in PurgeDeletionWaiters
        //delete inst;
    }
    //waitingDestruction.clear();

    // execute code!
    context.PushContext(); // first layer of context
    context.FlagCurrentDepth();

    // process awaiting instructions
    std::unordered_set<Instruction*> topbrosfromwait;
    topbrosfromwait.reserve(waiting.size()); // little optimisation
    for (auto it = waiting.begin(); it != waiting.end();) {
        topbrosfromwait.insert(it->first->GetFirstBro());
        context.PushContext(it->second);
        Instruction* from = it->first;
        it = waiting.erase(it);
        ExecuteFrom(from, code);
    }

    for (auto bro : code.bigBrothers) {
        // don't execute if one of our instruction were actually waiting
        if ((bro->model.flags & InstructionModel::Flags::Trigger) && topbrosfromwait.find(bro) == topbrosfromwait.end()) {
            ExecuteFrom(bro, code);
        }
    }
    context.PopContext();
    assert(context.IsEmpty() && "Something went wrong, some instruction forgot to pop the context maybe.");
}

void InstructionFamily::PurgeDeletionWaiters()
{
    for (auto inst : waitingDestruction) {
        delete inst;
    }
    waitingDestruction.clear();
}

void InstructionFamily::promoteToBigBro(Instruction* tr) {
    CodeSpace* parent = dynamic_cast<CodeSpace*>(tr->Parent());
    assert(parent && "Instructions must be dropped into CodeSpace types!");
    parent->bigBrothers.insert(tr);
    codeSpaces.insert(parent);
    //bigBrothers.insert(tr);
}

void InstructionFamily::demoteFromBigBro(Instruction* tr) {
    //auto it = std::find(bigBrothers.begin(), bigBrothers.end(), tr);
    //if (it != bigBrothers.end()) {
    //    bigBrothers.erase(it);
    //}
    for (auto space : codeSpaces) {
        space->bigBrothers.erase(tr);
    }
    //bigBrothers.erase(tr);
}

void InstructionFamily::orphanParameter(Instruction* tr) {
    orphanedParameters.insert(tr);
}

void InstructionFamily::unorphanParameter(Instruction* tr) {
    //auto it = std::find(orphanedParameters.begin(), orphanedParameters.end(), tr);
    //if (it != orphanedParameters.end()) {
    //    orphanedParameters.erase(it);
    //}
    orphanedParameters.erase(tr);
}

void InstructionFamily::DestroyInstruction(Instruction* tr)
{
    demoteFromBigBro(tr);
    unorphanParameter(tr);
    waitingDestruction.insert(tr);
}
