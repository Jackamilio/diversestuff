#include <iostream>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

#include "Scene.h"
#include "TextRect.h"
#include "TextRectFamily.h"

#include "Window.h"
#include "Button.h"

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

        TextRectFamily family;

        Window windowTest;
        windowTest.tl = glm::ivec2(200, 40);
        windowTest.resize(400, 300);

        gui.AddChild(&windowTest);

        Window windowTest2;
        windowTest2.tl = glm::ivec2(150, 20);
        windowTest2.resize(200, 150);

        gui.AddChild(&windowTest2);

        Button buttonTest;
        buttonTest += glm::ivec2(40, 40);
        buttonTest.resize(35, 35);

        windowTest2.AddChild(&buttonTest);
        
        //gui.AddDropLocation<TextRect>(gui);
        gui.AddDropLocation<TextRect>(windowTest);
        gui.AddDropLocation<TextRect>(windowTest2);

        ALLEGRO_FONT* font = fetchDefaultFont();

        TextRect tr(font, family);
        tr.SetText("Hello World!");
        tr.Place(20, 20);

        TextRect tr2(font, family);
        tr2.SetText("Hey I'm a brother.");
        tr2.Place(50, 50);

        TextRect tr3(font, family);
        tr3.SetText("Hello I'm a sibling as well!!");
        tr3.Place(30, 100);

        TextRect tr4(font, family);
        tr4.SetText("Hai caramba, holà que tal");
        tr4.Place(30, 150);

        // todo: have a special add for droppables
        windowTest.AddChild(&tr);
        windowTest.AddChild(&tr2);
        windowTest.AddChild(&tr3);
        windowTest.AddChild(&tr4);

        family.promoteToBigBro(&tr);
        family.promoteToBigBro(&tr2);
        family.promoteToBigBro(&tr3);
        family.promoteToBigBro(&tr4);

        while (engine.OneLoop()) {}

        GuiMaster::End();
    }
}
