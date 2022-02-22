#ifndef __INSTRUCTION_FAMILY_H__
#define __INSTRUCTION_FAMILY_H__

#include <vector>
#include <algorithm>
#include <allegro5/allegro5.h>
#include "Instruction.h"
#include "Cropper.h"
#include "InstructionContext.h"

typedef std::set<Instruction*> InstructionSet;

class CodeSpace : virtual public GuiElement {
public:
    InstructionSet bigBrothers;

    virtual ~CodeSpace();
};

class CodeInstance {
    friend class InstructionFamily;
private:
    InstructionSet& bigBrothers;
    std::unordered_map<Instruction*, InstructionContext> awaitingInstructions;
public:
    CodeInstance(CodeSpace& space) : bigBrothers(space.bigBrothers) {}
};

class InstructionFamily
{
    OTN(InstructionFamily);
private:
    typedef std::set<CodeSpace*> CodeSpaceSet;
    CodeSpaceSet codeSpaces;
    //InstructionSet bigBrothers;
    InstructionSet orphanedParameters;
    InstructionSet waitingDestruction;

    InstructionContext context;

    void ExecuteFrom(Instruction* inst, CodeInstance& code);

public:
    class Iterator {
    private:
        CodeSpaceSet::iterator spaceIt, endSpaceIt;
        InstructionSet::iterator bigBroIt;
        Instruction* curBro;

    public:
        Iterator(CodeSpaceSet& css);
        Iterator(CodeSpaceSet& css, int);

        bool operator !=(const Iterator& r) const;
        void operator ++();
        Instruction* operator*();
    };

    Iterator begin();
    Iterator end();

    void BuildParameterStack(std::stack<Instruction*>& out);

    InstructionFamily(ALLEGRO_FONT* font);
    ~InstructionFamily();

    void ExecuteCode(CodeInstance& code);
    void PurgeDeletionWaiters(); // MUST be called when all ExecuteCode calls are done this frame

    void promoteToBigBro(Instruction* tr);
    void demoteFromBigBro(Instruction* tr);

    void orphanParameter(Instruction* tr);
    void unorphanParameter(Instruction* tr);

    void DestroyInstruction(Instruction* tr);

    ALLEGRO_FONT* font;

    InstructionModel* emptyParameter = nullptr;
    Instruction* displacedBro = nullptr;
    Instruction* highlightedParam = nullptr;

    bool shadowBro = false;
    glm::ivec2 shadowBroPos;
};

#endif //__INSTRUCTION_FAMILY_H__