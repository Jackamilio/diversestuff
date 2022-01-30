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
        scene.tl = glm::ivec2(100, 100);
        scene.resize(540, 420);

        gui.AddChild(&scene);

        PureDisplacer pure;
        gui.AddChild(&pure, GuiElement::Priority::Bottom);

        Button buttontest;
        buttontest.tl = glm::ivec2(350, 50);
        buttontest.resize(20, 20);

        pure.AddChild(&buttontest, GuiElement::Priority::Bottom);
        
        gui.AddDropLocation<Instruction>(instructionsList);
        gui.AddDropLocation<Instruction>(pure);

        ALLEGRO_FONT* font = fetchDefaultFont();

        InstructionFamily family;

        InstructionModel tr(font, family);
        
        tr.SetText("Hello World!");
        tr.Place(20, 20);

        InstructionModel tr2(font, family);
        tr2.SetText("Hey I'm a brother.");
        tr2.Place(20, 50);

        InstructionModel tr3(font, family);
        tr3.SetText("Hello I'm a sibling as well!!");
        tr3.Place(20, 80);

        InstructionModel tr4(font, family);
        tr4.SetText("Hai caramba, holà que tal");
        tr4.Place(20, 110);

        instructionsList.AddChild(&tr);
        instructionsList.AddChild(&tr2);
        instructionsList.AddChild(&tr3);
        instructionsList.AddChild(&tr4);

        while (engine.OneLoop()) {}

        GuiMaster::End();
    }
}
