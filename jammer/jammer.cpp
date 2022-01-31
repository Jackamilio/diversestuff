#include <iostream>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

#include "Scene.h"
#include "Instruction.h"
#include "InstructionFamily.h"
#include "InstructionModel.h"

#include "Window.h"
#include "Button.h"
#include "PureDisplacer.h"

#include "GuiMaster.h"

#include <functional>

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
    float direction;

    SpriteTest(const char* image_file) :
        texture(gui.engine.graphics.textures.Get(image_file)),
        position{},
        direction(0.0f)
    {
    }

    virtual void Draw() {
        glm::vec2 center(texture.GetWidth(), texture.GetHeight());
        center *= 0.5f;
        al_draw_rotated_bitmap(texture.GetAlValue(), center.x, center.y, position.x, position.y, direction, 0);
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

        ALLEGRO_FONT* font = fetchDefaultFont();

        InstructionFamily family;

        int yoffset = 20;
        InstructionModel* curmodel;
        std::vector<InstructionModel*> deletelater;
        auto newmodel = [&font, &family, &curmodel, &yoffset, &instructionsList, &deletelater](const char* name) {
            curmodel = new InstructionModel(font, family);
            curmodel->SetText(name);
            curmodel->Place(20, yoffset);
            yoffset += 20;
            instructionsList.AddChild(curmodel);
            deletelater.push_back(curmodel);
        };

        newmodel("Avancer un peu");
        curmodel->function = [&sprite]() {
            sprite.position.x += 0.2f;
            return true;
        };

        newmodel("Tourner à gauche");
        curmodel->function = [&sprite]() {
            sprite.direction -= 0.01f;
            return true;
        };

        newmodel("Tourner à droite");
        curmodel->function = [&sprite]() {
            sprite.direction += 0.01f;
            return true;
        };

        newmodel("Je déclenche tout yo");
        curmodel->isTrigger = true;
        curmodel->function = []() {return true; };

        newmodel("Stop ce mf script");
        curmodel->function = []() {return false; };

        newmodel("Reviens par là, le singe!");
        curmodel->function = [&sprite]() {
            sprite.position = glm::vec2(270, 210);
            sprite.direction = 0.0f;
            return true;
        };

        while (engine.OneLoop()) {}

        for (auto inst : deletelater) {
            delete inst;
        }

        GuiMaster::End();
    }
}
