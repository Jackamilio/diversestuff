#include "EditableText.h"
#include "DefaultColors.h"
#include <allegro5/allegro_ttf.h>
#include <assert.h>

EditableText::EditableText(const char* starttext, int framepadding) :
    text(al_ustr_new(starttext)),
    cursorPos(0),
    framepadding(framepadding)
{
    ReFrame();
    CalculateCursorDraw();
}
EditableText::~EditableText() {
    al_ustr_free(text);
}

void EditableText::ReFrame() {
    if (al_ustr_size(text)) {
        MinimalFrame(*this);
        ALLEGRO_FONT* font = Font();
        al_do_multiline_ustr(font, 99999, text,
            [](int line_num, const ALLEGRO_USTR* line, void* caller) {
                EditableText& me = *(EditableText*)caller;
                Rect r;
                al_get_ustr_dimensions(me.Font(), line, &r.left, &r.top, &r.right, &r.bottom);
                r += glm::ivec2(0, line_num * al_get_font_line_height(me.Font()));
                me.mergeWith(r);
                return true;
            }
        , (void*)this);
        expand(framepadding);
    }
}

void EditableText::CalculateCursorDraw() {
    if (!al_ustr_size(text)) return;

    struct S {
        int cp;
        int cpt;
        glm::ivec2 p;
        ALLEGRO_FONT* font;
    };

    S s;
    s.cp = 0;
    s.cpt = cursorPos;
    s.p = glm::ivec2();
    s.font = Font();

    al_do_multiline_ustr(Font(), 99999, text,
        [](int line_num, const ALLEGRO_USTR* line, void* _s) {
            S& s = *(S*)_s;
            int l = (int)al_ustr_size(line) + 1;
            if (s.cp + l > s.cpt) {
                int ustrpos = al_ustr_offset(line, 0);
                int cp1 = ALLEGRO_NO_KERNING;
                int cp2 = ALLEGRO_NO_KERNING;
                bool cont = (ustrpos >= 0);
                while (cont) {
                    cp1 = cp2;
                    cp2 = al_ustr_get(line, ustrpos);
                    cont = al_ustr_next(line, &ustrpos);
                    s.p.x += al_get_glyph_advance(s.font, cp1, cp2);
                    if (s.cp + ustrpos > s.cpt) {
                        return false;
                    }
                }
                return false;
            }
            s.cp += l;
            s.p.y += al_get_font_line_height(s.font);
            return true;
        }
    , (void*)&s);

    cursorDraw.topleft = s.p;
    cursorDraw.resize(1, al_get_font_ascent(Font()));
}
void EditableText::CalculateCursorPos(const glm::ivec2 fromlocalpos) {
    struct S {
        int cp;
        glm::ivec2 p;
        ALLEGRO_FONT* font;
        int prev_l;
        const ALLEGRO_USTR* lastline;

        bool CalcLine(const ALLEGRO_USTR* line) {
            lastline = nullptr;
            int ustrpos = al_ustr_offset(line, 0);
            int prevustrpos = ustrpos;
            int cp1 = ALLEGRO_NO_KERNING;
            int cp2 = ALLEGRO_NO_KERNING;
            int px = 0;
            bool cont = (ustrpos >= 0);
            while (cont) {
                cp1 = cp2;
                cp2 = al_ustr_get(line, ustrpos);
                px += al_get_glyph_advance(font, cp1, cp2);
                if (px >= p.x) {
                    cp += prevustrpos;
                    return false;
                }
                prevustrpos = ustrpos;
                cont = al_ustr_next(line, &ustrpos);
            }
            cp += prevustrpos;
            return false;
        };
    };

    S s;
    s.cp = 0;
    s.p = fromlocalpos;
    s.font = Font();
    s.prev_l = 0;
    s.lastline = nullptr;

    al_do_multiline_ustr(Font(), 99999, text,
        [](int line_num, const ALLEGRO_USTR* line, void* _s) {
            S& s = *(S*)_s;
            s.cp += s.prev_l;
            int l = (int)al_ustr_size(line) + 1;
            if (s.p.y < al_get_font_line_height(s.font) * (line_num + 1)) {
                return s.CalcLine(line);
            }
            s.prev_l = l;
            s.lastline = line;
            return true;
        }
    , (void*)&s);

    if (s.lastline) s.CalcLine(s.lastline);

    cursorPos = s.cp;
}

void EditableText::SetText(const char* t)
{
    assert(al_ustr_assign_cstr(text, t) && "Something went wrong.");
    ReFrame();
    CalculateCursorDraw();
}

void EditableText::DrawFrame()
{
    (*this + Pos()).draw_outlined(black, grey, 1);
}

void EditableText::DrawText()
{
    const glm::ivec2& pos = Pos();
    al_draw_multiline_ustr(Font(), white, pos.x, pos.y, 99999, 0, 0, text);
}

void EditableText::DrawCursor()
{
    (cursorDraw + Pos()).draw_filled(white);
}

void EditableText::Draw() {
    DrawFrame();
    DrawText();
    DrawCursor();
}
Engine::InputStatus EditableText::Event(ALLEGRO_EVENT& event) {
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        glm::ivec2 mpos(event.mouse.x, event.mouse.y);
        mpos -= Pos();
        if (isInside(mpos)) {
            CalculateCursorPos(mpos);
            CalculateCursorDraw();
            MarkUDX();
        }
    }
    else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
        switch (event.keyboard.keycode) {
        case ALLEGRO_KEY_LEFT:
            if (DecrementCursor()) {
                CalculateCursorDraw();
                MarkUDX();
            }
            break;
        case ALLEGRO_KEY_RIGHT:
            if (IncrementCursor()) {
                CalculateCursorDraw();
                MarkUDX();
            }
            break;
        case ALLEGRO_KEY_UP:
            CalculateCursorPos(glm::ivec2(udx, cursorDraw.bottom - al_get_font_line_height(Font())));
            CalculateCursorDraw();
            break;
        case ALLEGRO_KEY_DOWN:
            CalculateCursorPos(glm::ivec2(udx, cursorDraw.bottom + al_get_font_line_height(Font())));
            CalculateCursorDraw();
            break;
        case ALLEGRO_KEY_ENTER:
            if (al_ustr_insert_chr(text, cursorPos, '\n')) {
                IncrementCursor();
                CalculateCursorDraw();
                MarkUDX();
                ReFrame();
            }
            break;
        case ALLEGRO_KEY_BACKSPACE:
            if (DecrementCursor()) {
                CalculateCursorDraw();
                MarkUDX();
                if (al_ustr_remove_chr(text, cursorPos)) ReFrame();
            }
            break;
        case ALLEGRO_KEY_DELETE:
            if (al_ustr_remove_chr(text, cursorPos)) {
                ReFrame();
            }
            break;
        case ALLEGRO_KEY_HOME:
        {
            int cp = cursorPos;
            while (DecrementCursor() && al_ustr_get(text, cursorPos) != '\n') {}
            if (cursorPos > 0) IncrementCursor();
            if (cp != cursorPos) {
                CalculateCursorDraw();
                MarkUDX();
            }
            break;
        }
        case ALLEGRO_KEY_END:
        {
            int cp = cursorPos;
            while (al_ustr_get(text, cursorPos) != '\n' && IncrementCursor()) {}
            if (cp != cursorPos) {
                CalculateCursorDraw();
                MarkUDX();
            }
            break;
        }
        default:
            if (event.keyboard.unichar > 0) {
                if (al_ustr_insert_chr(text, cursorPos, event.keyboard.unichar)) {
                    IncrementCursor();
                    CalculateCursorDraw();
                    MarkUDX();
                    ReFrame();
                }
            }
            else {
                return Engine::InputStatus::ignored;
            }
            break;
        }
        return Engine::InputStatus::grabbed;
    }
    return Engine::InputStatus::ignored;
}

EditableTextBox::EditableTextBox(ALLEGRO_FONT* font, int framepadding) :
    EditableText("", framepadding),
    pos{},
    font(font)
{
}

ALLEGRO_FONT* EditableTextBox::Font() const
{
    return font;
}

const glm::ivec2& EditableTextBox::Pos() const
{
    return pos;
}

void EditableTextBox::MinimalFrame(Rect& inout)
{
    inout = glm::ivec4(0, 0, 20, 20);
}
