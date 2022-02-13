#ifndef __EDITABLE_TEXT_H__
#define __EDITABLE_TEXT_H__

#include <allegro5/allegro_font.h>
#include "Rect.h"
#include "GuiElement.h"

class EditableText : virtual public Rect, virtual public GuiElement {
private:
    ALLEGRO_USTR* text;
    int cursorPos;
    Rect cursorDraw;
    int udx; // remembered x position when navigating up and down

    int numberPrecision;
public:
    int framepadding;
    bool canNewLine;

    virtual ALLEGRO_FONT* Font() const = 0;
    virtual const glm::ivec2& Pos() const = 0;
    virtual void MinimalFrame(Rect& inout) = 0;

    inline bool IncrementCursor() {
        return al_ustr_next(text, &cursorPos);
    }

    inline bool DecrementCursor() {
        return al_ustr_prev(text, &cursorPos);
    }

    inline void MarkUDX() {
        udx = cursorDraw.right;
    }

    EditableText(const char* starttext, int framepadding = 0);
    virtual ~EditableText();

    void Init(); // Don't forget to call this in subclasses, because Font() and MinimalFrame() can't be used in the constructor

    void ReFrame();

    void CalculateCursorDraw();
    bool CalculateCursorPos(const glm::ivec2 fromlocalpos); // returns if the position has changed

    inline const ALLEGRO_USTR* GetText() const { return text; }
    void SetText(const char* t);

    void SetNumerical(bool activate, int precision = 2);

    void DrawFrame();
    void DrawText();
    void DrawCursor();
    void Draw();
    Engine::InputStatus Event(ALLEGRO_EVENT& event);
};

class EditableTextBox : public EditableText {
public:
    glm::ivec2 pos;
    ALLEGRO_FONT* font;

    EditableTextBox(ALLEGRO_FONT* font, int framepadding = 0);

    ALLEGRO_FONT* Font() const;
    const glm::ivec2& Pos() const;
    void MinimalFrame(Rect& inout);
};

#endif //__EDITABLE_TEXT_H__