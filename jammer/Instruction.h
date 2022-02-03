#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Droppable.h"

class InstructionModel;

class Instruction : public Droppable<Instruction> {
    friend class InstructionFamily;
private:
    union {
        Instruction* bigBro;
        Instruction* owner;
    };
    
    Instruction* littleBro;

    bool isUnderBro(const Instruction& bro, bool checkAdjusted = false);
    bool isAboveBro(const Instruction& bro, bool checkAdjusted = false);

    void placeUnderBigBroRecursive();
    void placeAboveLittleBroRecursive();

    Instruction* getLastBro();

    Instruction(InstructionModel& model);
    ~Instruction();
public:
    glm::ivec2 pos;
    InstructionModel& model;
    std::vector<Instruction*> parameters;

    inline static Instruction* Create(InstructionModel& model) {
        return new Instruction(model);
    }
    inline void Destroy();

    void Draw();

    virtual void GrabbedBis();
    virtual void DroppedBis();
    virtual void DroppedBack();
    virtual void Dragged(const glm::ivec2& delta);

    virtual bool hitCheck(const glm::ivec2& pos) const;
    virtual void SetPos(const glm::ivec2& tsl);
    virtual glm::ivec2 GetPos() const;

    glm::ivec2 GetAdjustedPos() const;
};

#endif //__INSTRUCTION_H__