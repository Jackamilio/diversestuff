#include <iostream>
#include <functional>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

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

class FPSCounter : public Engine::Graphic, public SceneObject<FPSCounter> {
public:
    OTN(FPSCounter);
    IMPLEMENT_EXPOSE{
        EXPOSE_VALUE(samplingDuration);
    }

    float samplingDuration;
    double lastTime;
    int nbSamples;
    int lastFps;

    int ypos;

    FPSCounter() {
        engine.overlayGraphic.AddChild(this);

        samplingDuration = 0.5;
        lastTime = engine.time;
        nbSamples = 0;
        lastFps = (int)(1.0 / engine.dtTarget);

        static int nextypos = 40;
        ypos = nextypos;
        nextypos += 15;
    }
    ~FPSCounter() {
        engine.overlayGraphic.RemoveChild(this);
    }

    void Draw() {

        ++nbSamples;
        if (engine.time - lastTime > (double)samplingDuration) {
            lastFps = (int)((float)nbSamples / samplingDuration);
            nbSamples = 0;
            lastTime = engine.time;
        }

        al_draw_textf(fetchDefaultFont(), al_map_rgb(255, 255, 255), 10, ypos, 0, "FPS : %i", lastFps);
    }
};

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

int main()
{
    Engine& engine = Engine::Get();

    al_init_primitives_addon();

    if (engine.Init()) {
        GuiMaster::Init();

        GuiMaster& gui = GuiMaster::Get();

        Engine::Engine::Get().inputRoot.AddChild(&gui);
        Engine::Engine::Get().overlayGraphic.AddChild(&gui);

        Window instructionsList;
        instructionsList.tl = glm::ivec2(1, 20);
        instructionsList.resize(300, 719);

        gui.AddChild(&instructionsList);

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
        InstructionModel* curmodel;
        std::vector<InstructionModel*> deletelater;
        auto newmodel = [&](const char* name, InstructionModel::Type type = InstructionModel::Type::Default, int nbparams = 0) {
            curmodel = new InstructionModel(family);
            curmodel->type = type;
            curmodel->parametersTaken = nbparams;
            curmodel->SetText(name);
            curmodel->Place(20, yoffset);
            yoffset += 25;
            instructionsList.AddChild(curmodel);
            deletelater.push_back(curmodel);
        };

        newmodel("Avancer un peu");
        curmodel->function = [&sprite](Parameter*) {
            sprite.position.x += 0.2f;
            return true;
        };

        newmodel("Avancer de : ", InstructionModel::Type::Default, 1);
        curmodel->function = [&sprite](Parameter* p) {
            sprite.position.x += 0.2f * p[0];
            return true;
        };

        newmodel("Tourner à gauche");
        curmodel->function = [&sprite](Parameter*) {
            sprite.direction -= 0.01f;
            return true;
        };

        newmodel("Tourner à droite");
        curmodel->function = [&sprite](Parameter*) {
            sprite.direction += 0.01f;
            return true;
        };

        newmodel("Je déclenche tout yo");
        curmodel->isTrigger = true;
        curmodel->function = [](Parameter*) {return true; };

        newmodel("Stop ce mf script");
        curmodel->function = [](Parameter*) {return false; };

        newmodel("Reviens par là, le singe!");
        curmodel->function = [&sprite](Parameter*) {
            sprite.position = glm::vec2(270, 210);
            sprite.direction = 0.0f;
            return true;
        };

        newmodel("Mirroir + Renversé", InstructionModel::Type::Default, 2);
        curmodel->function = [&sprite](Parameter* pl) {
            sprite.scale.x = pl[0] ? -1 : 1;
            sprite.scale.y = pl[1] ? -1 : 1;
            return true;
        };

        newmodel("VRAI", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*) {
            return 1.0f;
        };

        newmodel("FAUX", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*) {
            return 0.0f;
        };

        newmodel("1", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*) { return 1.0f; };

        newmodel("2", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*) { return 2.0f; };

        newmodel("3", InstructionModel::Type::Parameter);
        curmodel->evaluate = [](Parameter*) { return 3.0f; };

        newmodel("Additionner", InstructionModel::Type::Parameter, 2);
        curmodel->evaluate = [](Parameter* p) {
            return p[0] + p[1];
        };

        newmodel("Multiplier par 10", InstructionModel::Type::Parameter, 1);
        curmodel->evaluate = [](Parameter* p) {
            return p[0] * 10.0f;
        };

        while (engine.OneLoop()) {}

        for (auto inst : deletelater) {
            delete inst;
        }

        GuiMaster::End();
    }
}
