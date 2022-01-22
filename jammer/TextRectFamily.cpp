#include "TextRectFamily.h"
#include <allegro5/allegro_primitives.h>
#include "DefaultColors.h"

TextRectFamily::Iterator::Iterator(vector<TextRect*>& vec) : bigBroIt(vec.begin()), endBroIt(vec.end()), curBro(*bigBroIt) {}
TextRectFamily::Iterator::Iterator(vector<TextRect*>& vec, int dummy) : bigBroIt(vec.end()), endBroIt(vec.end()), curBro(nullptr) {}

const bool TextRectFamily::Iterator::operator !=(Iterator& r) {
    return bigBroIt != r.bigBroIt || curBro != r.curBro;
}

void TextRectFamily::Iterator::operator ++() {
    if (curBro->littleBro) {
        curBro = curBro->littleBro;
    }
    else {
        ++bigBroIt;
        curBro = bigBroIt != endBroIt ? *bigBroIt : nullptr;
    }
}

TextRect* TextRectFamily::Iterator::operator*() {
    return curBro;
}

TextRectFamily::Iterator TextRectFamily::begin() {
    return Iterator(bigBrothers);
}

TextRectFamily::Iterator TextRectFamily::end() {
    return Iterator(bigBrothers, 0);
}

TextRectFamily::TextRectFamily() : shadowBroPos()
{
}

void TextRectFamily::Draw() {
    TextRect* draggedBro = dynamic_cast<TextRect*>(gui.CurrentDraggable());

    // shadow
    if (shadowBro && draggedBro)
    {
        Rect shadowRect = *draggedBro + shadowBroPos;
        shadowRect.draw_filled(lightgrey);
    }

    // big bros
    for (TextRect* bro : bigBrothers) {
        if (bro != draggedBro) {
            bro->Draw();
        }
    }

    // dragged bro above others
    if (draggedBro) {
        draggedBro->Draw();
    }
}

Engine::InputStatus TextRectFamily::Event(ALLEGRO_EVENT& event) {
    for (auto bro : bigBrothers) {
        if (bro->Event(event) == Engine::InputStatus::grabbed) {
            return Engine::InputStatus::grabbed;
        }
    }
    return Engine::InputStatus::ignored;
}

void TextRectFamily::promoteToBigBro(TextRect* tr) {
    bigBrothers.push_back(tr);
}

void TextRectFamily::demoteFromBigBro(TextRect* tr) {
    auto it = std::find(bigBrothers.begin(), bigBrothers.end(), tr);
    if (it != bigBrothers.end()) {
        bigBrothers.erase(it);
    }
}
