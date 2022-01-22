#include "TextRect.h"
#include "TextRectFamily.h"
#include "DefaultColors.h"
#include <allegro5/allegro_primitives.h>

const int dropL = 20;
const int dropR = 30;

bool TextRect::isUnderBro(const TextRect& bro) {
    //const int broleft = bro.x + bro.ox;
    //const int broDown = bro.y + bro.h;
    //return valueInside(x + ox, broleft - dropL, broleft + dropR) && valueInside(y + oy, broDown, broDown + bro.h - bro.oy);
    const glm::ivec2 brobl = bro.pos + bro.bl();
    const glm::ivec2 mytl = pos + tl;
    return valueInside(mytl.x, brobl.x - dropL, brobl.x + dropR) && valueInside(mytl.y, brobl.y, brobl.y + bro.h());
}

bool TextRect::isAboveBro(const TextRect& bro) {
    //const int broleft = bro.x + bro.ox;
    //const int broUp = bro.y;
    //return valueInside(x + ox, broleft - dropL, broleft + dropR) && valueInside(y + h, broUp - bro.h + bro.oy, broUp);
    const int broleft = bro.pos.x + bro.tl.x;
    const int broUp = bro.pos.y;
    const glm::ivec2 mybl = pos + bl();
    return valueInside(mybl.x, broleft - dropL, broleft + dropR) && valueInside(mybl.y, broUp - bro.h(), broUp);
}

void TextRect::placeUnderBigBroRecursive() {
    if (bigBro) {
        //x = bigBro->x + bigBro->ox - ox;
        //y = bigBro->y + bigBro->h - oy;
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
        //x = littleBro->x + littleBro->ox - ox;
        //y = littleBro->y - h + oy;
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
    Rect rect = *this + pos;
    rect.draw_filled(black);
    rect.draw(grey, 1);
    al_draw_text(font, white, pos.x, pos.y, 0, text);

    if (littleBro) littleBro->Draw();
}

void TextRect::Grabbed() {
    if (bigBro) {
        bigBro->littleBro = nullptr;
        bigBro = nullptr;
        family.promoteToBigBro(this);
    }
}

void TextRect::Dropped() {
    TextRect* lastBro = getLastBro();

    for (auto bro : family) {
        if (bro != this && isUnderBro(*bro)) {
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
        else if (bro != lastBro && !bro->bigBro && lastBro->isAboveBro(*bro)) {
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
}

void TextRect::Dragged(const glm::ivec2& delta) {
    TextRect* curBro = this;
    do {
        curBro->pos += delta;
        curBro = curBro->littleBro;
    } while (curBro);

    if (family.displacedBro && family.displacedBro->bigBro && !isUnderBro(*family.displacedBro->bigBro)) {
        family.displacedBro->placeUnderBigBroRecursive();
        family.displacedBro = nullptr;
    }

    if (!family.displacedBro) {
        for (auto bro : family) {
            if (bro != this && bro->littleBro && isUnderBro(*bro)) {
                family.displacedBro = bro->littleBro;
                bro->littleBro->pos.y += br.y - tl.y;
                if (bro->littleBro->littleBro) {
                    bro->littleBro->littleBro->placeUnderBigBroRecursive();
                }
            }
        }
    }

    family.shadowBro = false;
    TextRect* lastBro = getLastBro();
    for (auto bro : family) {
        if (bro != this && isUnderBro(*bro)) {
            family.shadowBro = true;
            //family.shadowBroX = bro->x + bro->ox - ox;
            //family.shadowBroY = bro->y + bro->h - oy;
            family.shadowBroPos = bro->pos;
            family.shadowBroPos.x += bro->tl.x - tl.x;
            family.shadowBroPos.y += bro->br.y - tl.y;
            break;
        }
        else if (bro != lastBro && !bro->bigBro && lastBro->isAboveBro(*bro)) {
            family.shadowBro = true;
            //family.shadowBroX = bro->x + bro->ox - lastBro->ox;
            //family.shadowBroY = bro->y - lastBro->h + lastBro->oy;
            family.shadowBroPos = bro->pos;
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

Engine::InputStatus TextRect::Event(ALLEGRO_EVENT& event) {
    if (Draggable::Event(event) == Engine::InputStatus::ignored) {
        return littleBro ? littleBro->Event(event) : Engine::InputStatus::ignored;
    }
    return Engine::InputStatus::grabbed;
}