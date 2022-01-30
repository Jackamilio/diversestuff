#ifndef __INSTRUCTION_FAMILY_H__
#define __INSTRUCTION_FAMILY_H__

#include <vector>
#include <algorithm>
#include <allegro5/allegro5.h>
#include "Instruction.h"
#include "CropperDisplacer.h"
#include "Engine.h"

using std::vector;

class InstructionFamily : public Engine::Update
{
    OTN(InstructionFamily);
private:
    vector<Instruction*> bigBrothers;
    vector<Instruction*> waitingDestruction;

public:
    class Iterator {
    private:
        vector<Instruction*>::iterator bigBroIt, endBroIt;
        Instruction* curBro;

    public:
        Iterator(vector<Instruction*>& vec);
        Iterator(vector<Instruction*>& vec, int dummy);

        bool operator !=(Iterator& r) const;
        void operator ++();
        Instruction* operator*();
    };

    Iterator begin();
    Iterator end();

    InstructionFamily();
    ~InstructionFamily();

    void Step();

    void promoteToBigBro(Instruction* tr);
    void demoteFromBigBro(Instruction* tr);

    inline void DestroyInstruction(Instruction* tr) {
        waitingDestruction.push_back(tr);
    }

    Instruction* displacedBro = nullptr;

    bool shadowBro = false;
    glm::ivec2 shadowBroPos;
};

#endif //__INSTRUCTION_FAMILY_H__