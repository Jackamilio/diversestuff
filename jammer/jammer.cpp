#include <iostream>
#include <functional>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "Scene.h"

#include "Instruction.h"
#include "InstructionFamily.h"
#include "InstructionModel.h"

#include "GuiMaster.h"
#include "Window.h"
#include "Button.h"
#include "PureDisplacer.h"

#include "DefaultColors.h"

ALLEGRO_FONT* fetchDefaultFont()
{
    static ALLEGRO_FONT* defaultFont = NULL;
    if (!defaultFont) {
        //	defaultFont = al_
        // _ttf_font("arial.ttf", 14, 0);
        //	if (!defaultFont) {
        defaultFont = al_create_builtin_font();
        //	}
    }
    return defaultFont;
}

class SpriteTest : public GuiElement {
public:
    const Texture& texture;
    glm::vec2 position;
    glm::vec2 scale;
    float direction;

    SpriteTest(const char* image_file) :
        texture(gui.engine.graphics.textures.Get(image_file)),
        position{},
        scale(1,1),
        direction(0.0f)
    {
    }

    virtual void Draw() {
        glm::vec2 center(texture.GetWidth(), texture.GetHeight());
        center *= 0.5f;
        al_draw_scaled_rotated_bitmap(texture.GetAlValue(), center.x, center.y, position.x, position.y, scale.x, scale.y, direction, 0);
    }
};

class EditableText : public Rect, public GuiElement {
public:
    ALLEGRO_USTR* text;
    ALLEGRO_FONT* font;
    glm::ivec2 pos;
    int cursorPos;
    Rect cursorDraw;
    int udx; // remembered x position when navigating up and down

    inline bool IncrementCursor() {
        return al_ustr_next(text, &cursorPos);
    }

    inline bool DecrementCursor() {
        return al_ustr_prev(text, &cursorPos);
    }

    inline void MarkUDX() {
        udx = cursorDraw.right;
    }

    EditableText(const char* starttext, ALLEGRO_FONT* font) :
        text(al_ustr_new(starttext)),
        font(font),
        pos{},
        cursorPos(0)
    {
        ReFrame();
        CalculateCursorDraw();
    }
    ~EditableText() {
        al_ustr_free(text);
    }

    void ReFrame() {
        ltrb = glm::ivec4();
        al_do_multiline_ustr(font, 99999, text,
            [](int line_num, const ALLEGRO_USTR* line, void* caller) {
                EditableText& me = *(EditableText*)caller;
                Rect r;
                al_get_ustr_dimensions(me.font, line, &r.left, &r.top, &r.right, &r.bottom);
                me.bottom += al_get_font_line_height(me.font);
                me.mergeWith(r);
                return true;
            }
        , (void*)this);
        bottom += top;
        expand(4);
    }

    void CalculateCursorDraw() {
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
        s.font = font;

        al_do_multiline_ustr(font, 99999, text,
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
        cursorDraw.resize(1, al_get_font_ascent(font));
    }
    void CalculateCursorPos(const glm::ivec2 fromlocalpos) {
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
        s.font = font;
        s.prev_l = 0;
        s.lastline = nullptr;

        al_do_multiline_ustr(font, 99999, text,
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

    void Draw() {
        (*this + pos).draw_outlined(black, grey, 1);
        al_draw_multiline_ustr(font, white, pos.x, pos.y, 99999, 0, 0, text);
        (cursorDraw + pos).draw_filled(white);
    }
    Engine::InputStatus Event(ALLEGRO_EVENT& event) {
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            glm::ivec2 mpos(event.mouse.x, event.mouse.y);
            mpos -= pos;
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
                CalculateCursorPos(glm::ivec2(udx, cursorDraw.bottom - al_get_font_line_height(font)));
                CalculateCursorDraw();
                break;
            case ALLEGRO_KEY_DOWN:
                CalculateCursorPos(glm::ivec2(udx, cursorDraw.bottom + al_get_font_line_height(font)));
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
                if (DecrementCursor() && al_ustr_remove_chr(text, cursorPos)) {
                    CalculateCursorDraw();
                    MarkUDX();
                    ReFrame();
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
};

int main()
{
    Engine& engine = Engine::Get();

    al_init_primitives_addon();
    al_init_ttf_addon();

    if (engine.Init()) {
        GuiMaster::Init();

        GuiMaster& gui = GuiMaster::Get();

        Engine::Engine::Get().inputRoot.AddChild(&gui);
        Engine::Engine::Get().overlayGraphic.AddChild(&gui);

        Window instructionsList;
        instructionsList.tl = glm::ivec2(1, 20);
        instructionsList.resize(300, 719);

        gui.AddChild(&instructionsList);

        ALLEGRO_FONT* arial = al_load_font("arial.ttf", 20, 0);

        EditableText texttest("Salut ça roule?\nJ'espère bien ouais.\nTroisième ligne.\nQUATRIEME LIGNe OUALALA", arial);
        texttest.pos = glm::ivec2(325, 25);
        gui.AddChild(&texttest);

        Window scene;
        scene.tl = glm::ivec2(700, 50);
        scene.resize(540, 420);

        gui.AddChild(&scene);

        SpriteTest sprite("cutemonkey.png");
        sprite.position = glm::vec2(270, 210);
        scene.AddChild(&sprite);

        PureDisplacer pure;
        gui.AddChild(&pure, GuiElement::Priority::Bottom);

        //Button buttontest;
        //buttontest.tl = glm::ivec2(350, 50);
        //buttontest.resize(20, 20);
        //pure.AddChild(&buttontest, GuiElement::Priority::Bottom);
        
        gui.AddDropLocation<Instruction>(instructionsList);
        gui.AddDropLocation<Instruction>(pure);

        InstructionFamily family(fetchDefaultFont());

        int yoffset = 20;
        InstructionModel* curmodel = nullptr;
        InstructionModel* previousmodel = nullptr;
        std::vector<InstructionModel*> deletelater;
        auto newmodel = [&](const char* name, InstructionModel::Type type = InstructionModel::Type::Default, int nbparams = 0) {
            previousmodel = curmodel;
            curmodel = new InstructionModel(family);
            curmodel->type = type;
            if (nbparams >= 0)
                curmodel->parametersTaken = nbparams;
            else
                curmodel->visible = false;
            curmodel->SetText(name);
            curmodel->Place(20, yoffset);
            if (curmodel->visible) yoffset += 25;
            instructionsList.AddChild(curmodel);
            deletelater.push_back(curmodel);
        };

        newmodel("Avancer un peu");
        curmodel->function = [&sprite](Parameter*, InstructionContext&) {
            sprite.position.x += 0.2f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Avancer de : ", InstructionModel::Type::Default, 1);
        curmodel->function = [&sprite](Parameter* p, InstructionContext&) {
            sprite.position.x += 0.2f * p[0];
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Tourner à gauche");
        curmodel->function = [&sprite](Parameter*, InstructionContext&) {
            sprite.direction -= 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Tourner à droite");
        curmodel->function = [&sprite](Parameter*, InstructionContext&) {
            sprite.direction += 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Je déclenche tout yo");
        curmodel->isTrigger = true;
        curmodel->function = [](Parameter*, InstructionContext&) {return InstructionModel::FunctionResult::Continue; };

        newmodel("Stop ce mf script");
        curmodel->function = [](Parameter*, InstructionContext&) {return InstructionModel::FunctionResult::Stop; };

        newmodel("Reviens par là, le singe!");
        curmodel->function = [&sprite](Parameter*, InstructionContext&) {
            sprite.position = glm::vec2(270, 210);
            sprite.direction = 0.0f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Mirroir + Renversé", InstructionModel::Type::Default, 2);
        curmodel->function = [&sprite](Parameter* pl, InstructionContext&) {
            sprite.scale.x = pl[0] ? -1 : 1;
            sprite.scale.y = pl[1] ? -1 : 1;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("VRAI", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, InstructionContext&) {
            return 1.0f;
        };

        newmodel("FAUX", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, InstructionContext&) {
            return 0.0f;
        };

        newmodel("NON", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p, InstructionContext&) {
            return *p != 0.0f ? 0.0f : 1.0f;
        };

        newmodel("1", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, InstructionContext&) { return 1.0f; };

        newmodel("10", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, InstructionContext&) { return 10.0f; };

        newmodel("Espace est appuyé", InstructionModel::Type::Parameter);
        curmodel->evaluate = [&engine](Parameter*, InstructionContext&) {
            return al_key_down(&engine.inputRoot.keyboardState, ALLEGRO_KEY_SPACE) ? 1 : 0;
        };

        newmodel("Additionner", InstructionModel::Type::Parameter, 2);
        curmodel->evaluate = [](Parameter* p, InstructionContext&) {
            return p[0] + p[1];
        };

        newmodel("Multiplier par 10", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p, InstructionContext&) {
            return p[0] * 10.0f;
        };

        newmodel("Si", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, InstructionContext&) {
            return p[0] != 0 ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        previousmodel->JumpsTo(curmodel);

        newmodel("Si", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, InstructionContext&) {
            return p[0] != 0 ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };
        InstructionModel* premiersi = curmodel;

        newmodel("[Caché] Skip le sinon", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, InstructionContext&) {
            return InstructionModel::FunctionResult::Jump;
        };
        previousmodel->Link(curmodel);
        InstructionModel* pasvupaspris = curmodel;

        newmodel("sinon", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        premiersi->JumpsTo(curmodel);

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        pasvupaspris->JumpsTo(curmodel);

        newmodel("Tant que", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, InstructionContext&) {
            return p[0] != 0 ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };
        InstructionModel* tantque = curmodel;

        newmodel("[Caché] Reboucle sur tant que", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, InstructionContext&) {
            return InstructionModel::FunctionResult::Jump | InstructionModel::FunctionResult::Await;
        };
        previousmodel->Link(curmodel);
        curmodel->JumpsTo(tantque);

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        tantque->JumpsTo(curmodel);

        newmodel("Répéter", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, InstructionContext& c) {
            c.PushContext();
            Parameter& counter = c.DeclareVariable("___repeat_counter___");
            counter = p[0];
            if (p[0] > 0) {
                return InstructionModel::FunctionResult::Continue;
            }
            return InstructionModel::FunctionResult::Jump;
        };
        InstructionModel* repeter = curmodel;

        newmodel("[Caché] jump target pour la boucle répéter", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        curmodel->stickToPrev = true;
        previousmodel->Link(curmodel);

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, InstructionContext& c) {
            Parameter& counter = c.GetVariable("___repeat_counter___");
            --counter;
            if (counter >= 1.0f) {
                return InstructionModel::FunctionResult::Jump | InstructionModel::FunctionResult::Await;
            }
            c.PopContext();
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        repeter->JumpsTo(curmodel);
        curmodel->JumpsTo(previousmodel);

        newmodel("Attendre", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, InstructionContext& c) {
            c.PushContext();
            Parameter& tt = c.DeclareVariable("___target_time___");
            tt = (float)al_get_time() + p[0];
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("[caché] attendre le temps", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, InstructionContext& c) {
            Parameter& tt = c.GetVariable("___target_time___");
            float curtime = (float)al_get_time();
            if (curtime < tt) {
                return InstructionModel::FunctionResult::Jump | InstructionModel::FunctionResult::Await;
            }
            c.PopContext();
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        curmodel->JumpsTo(curmodel); // jumps to itself, that's the trick!
        curmodel->stickToPrev = true;

        while (engine.OneLoop()) {}

        for (auto inst : deletelater) {
            delete inst;
        }

        GuiMaster::End();
    }
}
