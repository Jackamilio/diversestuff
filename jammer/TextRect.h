#ifndef __TEXT_RECT_H__
#define __TEXT_RECT_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Rect.h"
#include "Draggable.h"

class TextRectFamily;

class TextRect : public Rect, public Draggable {
    friend class TextRectFamily;
private:
    TextRectFamily& family;
    TextRect* bigBro;
    TextRect* littleBro;

    bool isUnderBro(const TextRect& bro);
    bool isAboveBro(const TextRect& bro);

    void placeUnderBigBroRecursive();
    void placeAboveLittleBroRecursive();

    TextRect* getLastBro();
public:
    glm::ivec2 pos;
    const char* text;
    ALLEGRO_FONT* font;

    TextRect(ALLEGRO_FONT* font, TextRectFamily& fam);

    inline void Place(int _x, int _y) {
        pos.x = _x;
        pos.y = _y;
    }

    void SetText(const char* t);

    void Draw();

    virtual void Grabbed();
    virtual void Dropped();
    virtual void Dragged(const glm::ivec2& delta);

    virtual bool hitCheck(const glm::ivec2& pos) const;

    virtual Engine::InputStatus Event(ALLEGRO_EVENT& event);
};

#endif //__TEXT_RECT_H__