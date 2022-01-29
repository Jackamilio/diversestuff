#include "InstructionFamily.h"
#include "InstructionModel.h"
#include "Instruction.h"
#include "DefaultColors.h"

InstructionModel::InstructionModel(ALLEGRO_FONT* font, InstructionFamily& fam) :
    family(fam),
    pos{},
    text(nullptr),
    font(font)
{
}

void InstructionModel::SetText(const char* t) {
    text = t;
    const int padding = 2;
    al_get_text_dimensions(font, text, &tl.x, &tl.y, &br.x, &br.y);
    tl.x -= padding;
    tl.y -= padding;
    br.x += padding;
    br.y += padding;
}

Engine::InputStatus InstructionModel::Event(ALLEGRO_EVENT& event)
{
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (isInside(glm::ivec2(event.mouse.x, event.mouse.y) - pos)) {
            // create an instruction and force grab it
            Instruction* newborn = Instruction::Create(*this);
            family.promoteToBigBro(newborn);
            newborn->SetPos(pos);
            newborn->ForceGrab();
            return Engine::InputStatus::grabbed;
        }
    }
    return Engine::InputStatus::ignored;
}

void InstructionModel::Draw()
{
    Draw(pos);
}

void InstructionModel::Draw(const glm::ivec2& pos) const
{
    Rect rect = *this + pos;
    rect.draw_filled(black);
    rect.draw(grey, 1);
    al_draw_text(font, white, pos.x, pos.y, 0, text);
}
