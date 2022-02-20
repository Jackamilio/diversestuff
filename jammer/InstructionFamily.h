#ifndef __INSTRUCTION_FAMILY_H__
#define __INSTRUCTION_FAMILY_H__

#include <vector>
#include <algorithm>
#include <allegro5/allegro5.h>
#include "Instruction.h"
#include "Cropper.h"
#include "Engine.h"
#include "InstructionContext.h"

using std::vector;

class InstructionFamily : public Engine::Update
{
    OTN(InstructionFamily);
private:
    vector<Instruction*> bigBrothers;
    vector<Instruction*> orphanedParameters;
    vector<Instruction*> waitingDestruction;

    InstructionContext context;

    std::unordered_map<Instruction*, InstructionContext> awaitingInstructions;

    void ExecuteFrom(Instruction* inst);

public:
    class Iterator {
    private:
        vector<Instruction*>::iterator bigBroIt, endBroIt;
        Instruction* curBro;

    public:
        Iterator(vector<Instruction*>& vec);
        Iterator(vector<Instruction*>& vec, int);

        bool operator !=(const Iterator& r) const;
        void operator ++();
        Instruction* operator*();
    };

    Iterator begin();
    Iterator end();

    void BuildParameterStack(std::stack<Instruction*>& out);

    InstructionFamily(ALLEGRO_FONT* font);
    ~InstructionFamily();

    void Step();

    void promoteToBigBro(Instruction* tr);
    void demoteFromBigBro(Instruction* tr);

    void orphanParameter(Instruction* tr);
    void unorphanParameter(Instruction* tr);

    inline void DestroyInstruction(Instruction* tr) {
        waitingDestruction.push_back(tr);
    }

    ALLEGRO_FONT* font;

    InstructionModel* emptyParameter = nullptr;
    Instruction* displacedBro = nullptr;
    Instruction* highlightedParam = nullptr;

    bool shadowBro = false;
    glm::ivec2 shadowBroPos;
};

#endif //__INSTRUCTION_FAMILY_H__