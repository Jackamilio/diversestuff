#include <iostream>
#include <functional>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>

#include "Scene.h"

#include "Instruction.h"
#include "InstructionFamily.h"
#include "InstructionModel.h"

#include "GuiMaster.h"
#include "Window.h"
#include "Button.h"
#include "PureDisplacer.h"

#include "DefaultColors.h"
#include "EditableText.h"
#include "Image.h"

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
    glm::vec2 scale;
    glm::vec2 pos;
    float direction;

    SpriteTest(const char* image_file) :
        texture(gui.engine.graphics.textures.Get(image_file)),
        scale(1,1),
        direction(0.0f)
    {
    }

    virtual void Draw() {
        glm::vec2 center(texture.GetWidth(), texture.GetHeight());
        center *= 0.5f;
        al_draw_scaled_rotated_bitmap(texture.GetAlValue(), center.x, center.y, pos.x, pos.y, scale.x, scale.y, direction, 0);
    }
};

class PureCodeSpace : public PureDisplacer, public CodeSpace {};

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

        std::vector<GuiElement*> deletelater;

        //Window resizeimagetest;
        //resizeimagetest.tl = { 20,20 };
        //resizeimagetest.resize(200, 200);
        //Image imagetest("banana.png");
        //imagetest.rect = &resizeimagetest;
        //resizeimagetest.AddChild(&imagetest);
        //gui.AddChild(&resizeimagetest, GuiElement::Priority::Top);
        //resizeimagetest.ReactTo(GuiElement::EventType::Moved, &resizeimagetest,
        //    [&resizeimagetest, &imagetest]() {imagetest.pos = -resizeimagetest.topleft; }
        //    );

        Window instructionsList;
        instructionsList.pos = { 1, 20 };
        instructionsList.resize(300, 719);

        gui.AddChild(&instructionsList);

        Window textureExplorer;
        textureExplorer.pos = {20 ,20};
        textureExplorer.resize(500, 350);

        engine.graphics.textures.Get("cutemonkey.png");
        engine.graphics.textures.Get("banana.png");
        engine.graphics.textures.Get("fail");

        std::vector<Button*> buttons;
        Button addImageButton;
        addImageButton.resize(120, 120);
        textureExplorer.AddChild(&addImageButton);
        addImageButton.ReactTo(GuiElement::EventType::Clicked, &addImageButton, [&engine, &textureExplorer]() {
            ALLEGRO_FILECHOOSER* dialog = al_create_native_file_dialog(
                nullptr,
                "Choose a new texture",
                "*.png;*.bmp;*.jpg;",
                ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
            if (al_show_native_file_dialog(engine.display, dialog)) {
                const char* answer = al_get_native_file_dialog_path(dialog, 0);
                engine.graphics.textures.Get(answer);
                textureExplorer.Fire(GuiElement::EventType::Resized);
            }
            al_destroy_native_file_dialog(dialog);
            });

        textureExplorer.ReactTo(GuiElement::EventType::Resized, &textureExplorer, [&engine, &deletelater, &buttons, &addImageButton, &textureExplorer]() {
            int x = 20;
            int y = 20;
            int i = 0;
            for (auto& tex : engine.graphics.textures.GetContainer()) {
                if (i < buttons.size()) {
                    GuiElement* firstchild = *buttons[i]->begin();
                    ((Image*)firstchild)->ChangeTexture(tex.second);
                }
                else {
                    Button* b = new Button;
                    Image* i = new Image(tex.second);
                    b->resize(120, 120);
                    b->AddChild(i);
                    i->rect = b;
                    buttons.push_back(b);
                    textureExplorer.AddChild(b);
                    deletelater.push_back(b);
                    deletelater.push_back(i);
                }
                ++i;
            }
            buttons.push_back(&addImageButton);
            for (auto button : buttons) {
                button->pos = { x, y };
                x += 140;
                if (x + button->w() > textureExplorer.w()) {
                    x = 20;
                    y += 140;
                }
            }
            buttons.erase(buttons.begin() + (buttons.size() - 1));
            });
        textureExplorer.Fire(GuiElement::EventType::Resized);

        //gui.AddChild(&textureExplorer);

        Window scene;
        scene.pos = { 700, 50 };
        scene.resize(540, 420);

        gui.AddChild(&scene);

        SpriteTest sprite("cutemonkey.png");
        sprite.pos = glm::vec2(270, 210);
        scene.AddChild(&sprite);

        PureCodeSpace pure;
        gui.AddChild(&pure, GuiElement::Priority::Bottom);

        CodeInstance code(pure);
        
        gui.AddDropLocation<Instruction>(instructionsList);
        gui.AddDropLocation<Instruction>(pure);

        InstructionFamily family(fetchDefaultFont());

        int yoffset = 20;
        InstructionModel* curmodel = nullptr;
        InstructionModel* previousmodel = nullptr;
        auto newmodel = [&](const char* name, InstructionModel::Type type = InstructionModel::Type::Default, int nbparams = 0) {
            previousmodel = curmodel;
            curmodel = new InstructionModel(family);
            curmodel->type = type;
            if (nbparams >= 0)
                curmodel->parametersTaken = nbparams;
            else
                curmodel->flags.unset(InstructionModel::Flags::Visible);
            curmodel->SetText(name);
            curmodel->pos = { 20, yoffset };
            if (curmodel->flags & InstructionModel::Flags::Visible) yoffset += 25;
            instructionsList.AddChild(curmodel);
            deletelater.push_back(curmodel);
        };

        newmodel("Avancer un peu");
        curmodel->function = [&sprite](Parameter*, const Instruction&, InstructionContext&) {
            sprite.pos.x += 0.2f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Avancer de : ", InstructionModel::Type::Default, 1);
        curmodel->function = [&sprite](Parameter* p, const Instruction&, InstructionContext&) {
            sprite.pos.x += 0.2f * p[0];
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Tourner à gauche");
        curmodel->function = [&sprite](Parameter*, const Instruction&, InstructionContext&) {
            sprite.direction -= 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Tourner à droite");
        curmodel->function = [&sprite](Parameter*, const Instruction&, InstructionContext&) {
            sprite.direction += 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Je déclenche tout yo");
        curmodel->flags |= InstructionModel::Flags::Trigger;
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {return InstructionModel::FunctionResult::Continue; };

        newmodel("Stop ce mf script");
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {return InstructionModel::FunctionResult::Stop; };

        newmodel("Reviens par là, le singe!");
        curmodel->function = [&sprite](Parameter*, const Instruction&, InstructionContext&) {
            sprite.pos = glm::vec2(270, 210);
            sprite.direction = 0.0f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Mirroir + Renversé", InstructionModel::Type::Default, 2);
        curmodel->function = [&sprite](Parameter* pl, const Instruction&, InstructionContext&) {
            sprite.scale.x = pl[0] ? -1 : 1;
            sprite.scale.y = pl[1] ? -1 : 1;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("VRAI", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, const Instruction&, InstructionContext&) {
            return 1.0f;
        };

        newmodel("FAUX", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, const Instruction&, InstructionContext&) {
            return 0.0f;
        };

        newmodel("NON", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p, const Instruction&, InstructionContext&) {
            return *p != 0.0f ? 0.0f : 1.0f;
        };

        newmodel("Espace est appuyé", InstructionModel::Type::Parameter);
        curmodel->evaluate = [&engine](Parameter*, const Instruction&, InstructionContext&) {
            return al_key_down(&engine.inputRoot.keyboardState, ALLEGRO_KEY_SPACE) ? 1 : 0;
        };

        newmodel("Additionner", InstructionModel::Type::Parameter, 2);
        curmodel->evaluate = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0] + p[1];
        };

        newmodel("Multiplier par 10", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0] * 10.0f;
        };

        newmodel("Si", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0] != 0 ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        previousmodel->JumpsTo(curmodel);

        newmodel("Si", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0] != 0 ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };
        InstructionModel* premiersi = curmodel;

        newmodel("[Caché] Skip le sinon", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Jump;
        };
        previousmodel->Link(curmodel);
        InstructionModel* pasvupaspris = curmodel;

        newmodel("sinon", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        premiersi->JumpsTo(curmodel);

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        pasvupaspris->JumpsTo(curmodel);

        newmodel("Tant que", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0] != 0 ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };
        InstructionModel* tantque = curmodel;

        newmodel("[Caché] Reboucle sur tant que", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Jump | InstructionModel::FunctionResult::Await;
        };
        previousmodel->Link(curmodel);
        curmodel->JumpsTo(tantque);

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        tantque->JumpsTo(curmodel);

        newmodel("Répéter", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext& c) {
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
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        curmodel->flags |= InstructionModel::Flags::StickToPrev;
        previousmodel->Link(curmodel);

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext& c) {
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
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext& c) {
            c.PushContext();
            Parameter& tt = c.DeclareVariable("___target_time___");
            tt = (float)al_get_time() + p[0];
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("[caché] attendre le temps", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext& c) {
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
        curmodel->flags |= InstructionModel::Flags::StickToPrev;

        while (engine.OneLoop()) {
            family.ExecuteCode(code);
            family.PurgeDeletionWaiters();
        }

        for (auto inst : deletelater) {
            delete inst;
        }

        GuiMaster::End();
    }
}
