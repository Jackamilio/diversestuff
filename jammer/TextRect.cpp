#include "TextRect.h"
#include "TextRectFamily.h"
#include "DefaultColors.h"
#include <allegro5/allegro_primitives.h>
#include "GuiMaster.h"

const int dropL = 20;
const int dropR = 30;

bool TextRect::isUnderBro(const TextRect& bro, bool checkAdjusted) {
    const glm::ivec2 brobl = (checkAdjusted ? bro.GetAdjustedPos() : bro.pos) + bro.bl();
    const glm::ivec2 mytl = pos + tl;
    return valueInside(mytl.x, brobl.x - dropL, brobl.x + dropR) && valueInside(mytl.y, brobl.y, brobl.y + bro.h());
}

bool TextRect::isAboveBro(const TextRect& bro, bool checkAdjusted) {
    //const int broleft = bro.pos.x + bro.tl.x;
    //const int broUp = bro.pos.y;
    glm::ivec2 bap = checkAdjusted ? bro.GetAdjustedPos() : bro.pos;
    const int broleft = bap.x + bro.tl.x;
    const int broUp = bap.y;
    const glm::ivec2 mybl = pos + bl();
    return valueInside(mybl.x, broleft - dropL, broleft + dropR) && valueInside(mybl.y, broUp - bro.h(), broUp);
}

void TextRect::placeUnderBigBroRecursive() {
    if (bigBro) {
        pos = bigBro->pos;
        pos.x += bigBro->tl.x - tl.x;
        pos.y += bigBro->h();

        if (littleBro) {
            littleBro->placeUnderBigBroRecursive();
        }
    }
}

void TextRect::placeAboveLittleBroRecursive() {
    if (littleBro) {
        pos = littleBro->pos;
        pos.x += littleBro->tl.x - tl.x;
        pos.y -= littleBro->h();

        if (bigBro) {
            bigBro->placeAboveLittleBroRecursive();
        }
    }
}

TextRect* TextRect::getLastBro() {
    TextRect* ret = this;
    while (ret->littleBro) {
        ret = ret->littleBro;
    }
    return ret;
}

TextRect::TextRect(ALLEGRO_FONT* font, TextRectFamily& fam) :
    family(fam),
    bigBro(nullptr),
    littleBro(nullptr),
    pos{},
    text(nullptr),
    font(font)
{}

void TextRect::SetText(const char* t) {
    text = t;
    const int padding = 2;
    al_get_text_dimensions(font, text, &tl.x, &tl.y, &br.x, &br.y);
    tl.x -= padding;
    tl.y -= padding;
    br.x += padding;
    br.y += padding;
}

void TextRect::Draw() {
    // shadow
    if (family.shadowBro && gui.CurrentDraggable() == this)
    {
        Rect shadowRect = *this + family.shadowBroPos;
        shadowRect.draw_filled(lightgrey);
    }

    // self
    Rect rect = *this + pos;
    rect.draw_filled(black);
    rect.draw(grey, 1);
    al_draw_text(font, white, pos.x, pos.y, 0, text);
}

void TextRect::GrabbedBis() {
    if (bigBro) {
        bigBro->littleBro = nullptr;
        bigBro = nullptr;
        family.promoteToBigBro(this);
    }

    TextRect* curBro = this->littleBro;
    while (curBro) {
        if (curBro->currentDropLocation) {
            curBro->currentDropLocation->Reject(curBro);
        }
        gui.AddChild(curBro);
        curBro->PutOnTop();
        curBro = curBro->littleBro;
    }
}

void TextRect::DroppedBis() {
    // accept all little bros into the new drop location
    TextRect* curLilBro = this->littleBro;
    while (curLilBro) {
        currentDropLocation->ForceAccept(curLilBro);
        curLilBro = curLilBro->littleBro;
    }

    TextRect* lastBro = getLastBro();
    for (auto bro : family) {
        if (bro != this && currentDropLocation == bro->currentDropLocation && isUnderBro(*bro)) {
            bigBro = bro;
            if (bro->littleBro) {
                lastBro->littleBro = bro->littleBro;
                bro->littleBro->bigBro = lastBro;
            }
            bro->littleBro = this;

            family.displacedBro = nullptr;
            placeUnderBigBroRecursive();
            family.demoteFromBigBro(this);
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
            family.demoteFromBigBro(bro);
            break;
        }
    }

    PutAtBottom();
}

void TextRect::DroppedBack()
{
    if (littleBro) {
        glm::ivec2 savepos = pos;
        pos += currentDropLocation->GetGlobalOffset();
        littleBro->placeUnderBigBroRecursive();
        pos = savepos;
    }
}

void TextRect::Dragged(const glm::ivec2& delta) {
    // move the bloc of bros
    TextRect* curBro = this;
    do {
        curBro->pos += delta;
        curBro = curBro->littleBro;
    } while (curBro);

    // displacement preview before dropping
    if (family.displacedBro && family.displacedBro->bigBro && !isUnderBro(*family.displacedBro->bigBro, true)) {
        family.displacedBro->placeUnderBigBroRecursive();
        family.displacedBro = nullptr;
    }

    if (!family.displacedBro) {
        for (auto bro : family) {
            if (bro != this && bro->littleBro && isUnderBro(*bro, true)) {
                family.displacedBro = bro->littleBro;
                bro->littleBro->pos.y += br.y - tl.y;
                if (bro->littleBro->littleBro) {
                    bro->littleBro->littleBro->placeUnderBigBroRecursive();
                }
            }
        }
    }

    // shadow position of the drop
    family.shadowBro = false;
    TextRect* lastBro = getLastBro();
    for (auto bro : family) {
        if (bro != this && isUnderBro(*bro, true)) {
            family.shadowBro = true;
            family.shadowBroPos = bro->GetAdjustedPos();
            family.shadowBroPos.x += bro->tl.x - tl.x;
            family.shadowBroPos.y += bro->br.y - tl.y;
            break;
        }
        else if (bro != lastBro && !bro->bigBro && lastBro->isAboveBro(*bro, true)) {
            family.shadowBro = true;
            family.shadowBroPos = bro->GetAdjustedPos();
            family.shadowBroPos.x += bro->tl.x - lastBro->tl.x;
            family.shadowBroPos.y -= lastBro->h();
            break;
        }
    }
}

bool TextRect::hitCheck(const glm::ivec2& opos) const
{
    return isInside(opos - pos);
}

void TextRect::SetPos(const glm::ivec2& tsl)
{
    pos = tsl;
}

glm::ivec2 TextRect::GetPos() const
{
    return pos;
}

glm::ivec2 TextRect::GetAdjustedPos() const
{
    if (currentDropLocation) {
        return currentDropLocation->GetGlobalOffset() + pos;
    }
    else {
        return pos;
    }
}
