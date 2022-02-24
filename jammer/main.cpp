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

#include "Jammer.h"

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

class JammerDisplay : public GuiElement {
public:
    const Jammer& jammer;

    JammerDisplay(const Jammer& j) : jammer(j)
    {
    }

    virtual void Draw() {
        for (auto sprite : jammer.liveScene.instances) {
            auto it = jammer.images.find(sprite.image);
            std::string imagefile = it != jammer.images.end() ? it->second : "errortex";
            const Texture& tex = gui.engine.graphics.textures.Get(imagefile);
            glm::vec2 center(tex.GetWidth(), tex.GetHeight());
            center *= 0.5f;
            ALLEGRO_COLOR color = al_map_rgba_f(sprite.color.r, sprite.color.g, sprite.color.b, sprite.color.a);
            al_draw_tinted_scaled_rotated_bitmap(tex.GetAlValue(), color, center.x, center.y, sprite.position.x, sprite.position.y, sprite.scale, sprite.scale, sprite.direction, 0);
        }
    }
};

class WindowCodeSpace : public Window, public CodeSpace {
public:
    WindowCodeSpace() : Cropper(*((Rect*)this)) {}
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

        Jammer JAMMER;
        JAMMER.images["monkey"] = "cutemonkey.png";
        JAMMER.images["banana"] = "banana.png";

        Jammer::Sprite sprite;
        sprite.position = {270.0f, 210.0f};
        sprite.image = "monkey";
        JAMMER.liveScene.instances.push_back(sprite);

        sprite.position = { 270.0f, 300.0f };
        sprite.image = "banana";
        sprite.scale = 0.2f;
        JAMMER.liveScene.instances.push_back(sprite);

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

        JammerDisplay jammerDisplay(JAMMER);
        scene.AddChild(&jammerDisplay);

        WindowCodeSpace codeSpace1;
        codeSpace1.pos = { 100,30 };
        codeSpace1.resize(540, 420);
        gui.AddChild(&codeSpace1);

        WindowCodeSpace codeSpace2;
        codeSpace2.pos = { 150,80 };
        codeSpace2.resize(540, 420);
        gui.AddChild(&codeSpace2);

        CodeInstance code1(codeSpace1);
        CodeInstance code2(codeSpace2);
        
        gui.AddDropLocation<Instruction>(instructionsList);
        gui.AddDropLocation<Instruction>(codeSpace1);
        gui.AddDropLocation<Instruction>(codeSpace2);

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
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext& c) {
            Jammer::Sprite& sprite = *c.GetVariable("___current_sprite___").sprite;
            sprite.position.x += 0.2f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Avancer de : ", InstructionModel::Type::Default, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext& c) {
            Jammer::Sprite& sprite = *c.GetVariable("___current_sprite___").sprite;
            sprite.position.x += 0.2f * p[0].fvalue;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Tourner à gauche");
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext& c) {
            Jammer::Sprite& sprite = *c.GetVariable("___current_sprite___").sprite;
            sprite.direction -= 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Tourner à droite");
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext& c) {
            Jammer::Sprite& sprite = *c.GetVariable("___current_sprite___").sprite;
            sprite.direction += 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Je déclenche tout yo");
        curmodel->flags |= InstructionModel::Flags::Trigger;
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {return InstructionModel::FunctionResult::Continue; };

        newmodel("Stop ce mf script");
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {return InstructionModel::FunctionResult::Stop; };

        newmodel("Reviens par là, le singe!");
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext& c) {
            Jammer::Sprite& sprite = *c.GetVariable("___current_sprite___").sprite;
            sprite.position = { 270, 210 };
            sprite.direction = 0.0f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("Changer la taille %", InstructionModel::Type::Default, 1);
        curmodel->function = [](Parameter* pl, const Instruction&, InstructionContext& c) {
            Jammer::Sprite& sprite = *c.GetVariable("___current_sprite___").sprite;
            sprite.scale = pl[0].fvalue * 0.01f;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("VRAI", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, const Instruction&, InstructionContext&) {
            return Parameter{ 1.0f };
        };

        newmodel("FAUX", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*, const Instruction&, InstructionContext&) {
            return Parameter{ 0.0f };
        };

        newmodel("NON", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p, const Instruction&, InstructionContext&) {
            return Parameter{ p->fvalue != 0.0f ? 0.0f : 1.0f };
        };

        newmodel("Espace est appuyé", InstructionModel::Type::Parameter);
        curmodel->evaluate = [&engine](Parameter*, const Instruction&, InstructionContext&) {
            return Parameter{ al_key_down(&engine.inputRoot.keyboardState, ALLEGRO_KEY_SPACE) ? 1.0f : 0.0f };
        };

        newmodel("Additionner", InstructionModel::Type::Parameter, 2);
        curmodel->evaluate = [](Parameter* p, const Instruction&, InstructionContext&) {
            return Parameter{ p[0].fvalue + p[1].fvalue };
        };

        newmodel("Multiplier par 10", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p, const Instruction&, InstructionContext&) {
            return Parameter{ p[0].fvalue * 10.0f };
        };

        newmodel("Si", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0].fvalue != 0.0f ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
        };

        newmodel("   ", InstructionModel::Type::Jump);
        curmodel->function = [](Parameter*, const Instruction&, InstructionContext&) {
            return InstructionModel::FunctionResult::Continue;
        };
        previousmodel->Link(curmodel);
        previousmodel->JumpsTo(curmodel);

        newmodel("Si", InstructionModel::Type::Jump, 1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext&) {
            return p[0].fvalue != 0.0f ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
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
            return p[0].fvalue != 0.0f ? InstructionModel::FunctionResult::Continue : InstructionModel::FunctionResult::Jump;
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
            if (p[0].fvalue > 0.0f) {
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
            float& counter = c.GetVariable("___repeat_counter___").fvalue;
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
            float& tt = c.DeclareVariable("___target_time___").fvalue;
            tt = (float)al_get_time() + p[0].fvalue;
            return InstructionModel::FunctionResult::Continue;
        };

        newmodel("[caché] attendre le temps", InstructionModel::Type::Jump, -1);
        curmodel->function = [](Parameter* p, const Instruction&, InstructionContext& c) {
            float& tt = c.GetVariable("___target_time___").fvalue;
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

        InstructionContext context;

        while (engine.OneLoop()) {
            context.PushContext();
            context.DeclareVariable("___current_sprite___");
            auto it = JAMMER.liveScene.instances.begin();
            context.GetVariable("___current_sprite___").sprite = &(*it);
            family.ExecuteCode(code1, context);
            ++it;
            context.GetVariable("___current_sprite___").sprite = &(*it);
            family.ExecuteCode(code2, context);
            family.PurgeDeletionWaiters();
            context.PopContext();
            assert(context.IsEmpty() && "Something went wrong, some instruction forgot to pop the context maybe.");
        }

        for (auto inst : deletelater) {
            delete inst;
        }

        GuiMaster::End();
    }
}
