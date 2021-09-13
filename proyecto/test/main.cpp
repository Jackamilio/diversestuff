#include "Engine.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Exposing.h"
#include <map>
#include "imgui.h"
#include "MapEditor.h"
#include "Scene.h"
#include "TemporaryOrTesting.h"

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

int main(int nbarg, char ** args) {
	Engine& engine = Engine::Get();

	engine.graphics.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	engine.graphics.pointLights[0][0] = glm::vec4(5.0f, 0.0f, 1.0f, 3.0f);
	engine.graphics.pointLights[0][1] = glm::vec4(0.7f, 0.0f, 0.0f, 0.0f);
	engine.graphics.pointLights[1][0] = glm::vec4(0.0f, 1.0f, 0.75f, 1.5f);
	engine.graphics.pointLights[1][1] = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);

	if (engine.Init()) {
		al_init_primitives_addon();
		al_init_ttf_addon();

		TestCamera camera;
		EngineMap map("niveau.lvl");
		MapEditor editor(map.mapdt, &map);
		TestModel tm("personnage.fbx", "Cours");
		TestCharacter tc;
		FPSCounter fc;

		camera.trackbody = tc.body;
		engine.Access<TestCamera*>() = &camera;

		while (engine.OneLoop()) {}
	}
}
