#ifndef __TEXT_RECT_H__
#define __TEXT_RECT_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Rect.h"
#include "Droppable.h"

class TextRectFamily;

class TextRect : public Rect, public Droppable<TextRect> {
    friend class TextRectFamily;
private:
    TextRectFamily& family;
    TextRect* bigBro;
    TextRect* littleBro;

    bool isUnderBro(const TextRect& bro, bool checkAdjusted = false);
    bool isAboveBro(const TextRect& bro, bool checkAdjusted = false);

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

    virtual void GrabbedBis();
    virtual void DroppedBis();
    virtual void DroppedBack();
    virtual void Dragged(const glm::ivec2& delta);

    virtual bool hitCheck(const glm::ivec2& pos) const;
    virtual void SetPos(const glm::ivec2& tsl);
    virtual glm::ivec2 GetPos() const;

    glm::ivec2 GetAdjustedPos() const;
};

#endif //__TEXT_RECT_H__