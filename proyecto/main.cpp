#include "Engine.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "LevelData.h"
#include "Dump.h"
#include "Editorcamera.h"
#include "Exposing.h"
#include <map>

#include "imgui.h"
#include "imgui_impl_allegro5.h"

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


class TestCamera : public Engine::Input, public Engine::Update {
public:
	Engine & engine;
	EditorCamera camera;
	//btRigidBody* trackbody;

	bool mleft;
	glm::vec4 mouse;

	TestCamera(Engine& e) : engine(e) {
		mleft = false;
		//trackbody = 0;

		engine.inputRoot.AddChild(this);
		engine.updateRoot.AddChild(this);

		camera.distance = 10.0f;
		camera.SetFocusPoint(0, 0, 0.5f);
		camera.SetUp(0, 0, 1);

		engine.graphics.proj = glm::perspective(glm::radians(40.0f), 640.0f / 480.0f, 0.5f, 50.0f);
	}

	bool Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			mouse.x = event.mouse.dx;
			mouse.y = event.mouse.dy;
			mouse.z = event.mouse.dz;
			mouse.w = event.mouse.dw;
		}

		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
			mleft = true;
			return true;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
			mleft = false;
			return true;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && mleft) {
			camera.horAngle += (float)event.mouse.dx * 0.01f;
			camera.verAngle += (float)event.mouse.dy * 0.01f;
			return true;
		}
		return false;
	}

	void Step() {

		/*if (trackbody) {
			btTransform tr = trackbody->getCenterOfMassTransform();
			camera.SetFocusPoint(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
		}*/

		camera.CalcMatrix(engine.graphics.view);

		//ImGui::Begin("Debug camera");
		//ImGui::SliderFloat("Distance", &camera.distance, 0.1f, 50.0f);
		//ImGui::End();

		ImGui::Begin("Last mouse");
		ImGui::InputFloat4("Values", glm::value_ptr(mouse));
		ImGui::End();
	}
};

class EngineLevel : public Engine::Graphic {
public:
	Engine & engine;
	LevelData lvldt;
	const Model* model;

	//btTriangleMesh* levelMesh;
	//btBvhTriangleMeshShape* shape;
	//btMotionState* motion;
	//btRigidBody* body;

	EngineLevel(Engine& e, const char* level) : engine(e) {
		lvldt.OldLoad(level);
		model = &engine.graphics.models.Get(&lvldt);
		DrawLevelData(lvldt, engine.graphics.textures, true);
		//glEndList();

		/*btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));
		levelMesh = ConstructLevelCollision(lvldt);
		shape = new btBvhTriangleMeshShape(levelMesh, true);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, shape);
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);*/

		engine.mainGraphic.AddChildForProgram(this, "test.pgr");
		//engine.mainGraphic.AddChild(this);
	}

	~EngineLevel() {
		//delete body;
		//delete motion;
		//delete shape;
		//delete levelMesh;
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4(1.0));
		model->Draw();
		//DrawLevelData(lvldt, engine.graphics.textures, true);

		// imgui image test
		ImGui::Begin("Image test");

		if (ImGui::TreeNode("Show")) {
			const Texture& tex = engine.graphics.textures.Get("tileset.png");
			ImGui::Image(tex.GetAlValue(), ImVec2(tex.GetWidth(), tex.GetHeight()));
			ImGui::TreePop();
		}

		ImGui::End();

		ImGui::ShowDemoWindow();
	}
};

class ExposingTestMember {
public:
	int _otherInt;
	short _otherShort;
	bool _otherBool;

	ExposingTestMember()
		: _otherInt(0)
		, _otherShort(0)
		, _otherBool(true)
	{}

	IM_AN_EXPOSER
};

#define EXPOSE_TYPE ExposingTestMember
EXPOSE_START
EXPOSE(_otherInt)
EXPOSE(_otherShort)
EXPOSE(_otherBool)
EXPOSE_END
#undef EXPOSE_TYPE

class ExposingTest {
public:
	bool _bool;
	char _char;
	unsigned char _uchar;
	short _short;
	unsigned short _ushort;
	int _int;
	unsigned int _uint;
	float _float;
	double _double;
	std::string _string;
	ExposingTestMember _member;
	std::vector<int> _vector;
	std::vector<ExposingTestMember> _vectorMember;
	std::map<std::string, int> _mapMember;

	struct NotExposed {
		int val;
	};

	NotExposed _notExposed;
	
	ExposingTest()
		: _bool(false)
		, _char(0)
		, _uchar(0)
		, _short(0)
		, _ushort(0)
		, _int(0)
		, _uint(0)
		, _float(0.0f)
		, _double(0.0)
		, _string("string")
	{
		_vector.push_back(0);
		_vector.push_back(1);

		ExposingTestMember member;
		_vectorMember.push_back(member);
		member._otherInt = 1;
		_vectorMember.push_back(member);
		member._otherBool = true;
		_vectorMember.push_back(member);

		_mapMember["one"] = 1;
		_mapMember["two"] = 2;
		_mapMember["three"] = 3;
	}

	IM_AN_EXPOSER
};

#define EXPOSE_TYPE ExposingTest
EXPOSE_START
EXPOSE(_mapMember)
EXPOSE(_bool)
EXPOSE(_char)
EXPOSE(_uchar)
EXPOSE(_short)
EXPOSE(_ushort)
EXPOSE(_member)
EXPOSE(_notExposed)
EXPOSE(_int)
EXPOSE(_uint)
EXPOSE(_float)
EXPOSE(_double)
EXPOSE(_string)
EXPOSE(_vector)
EXPOSE(_vectorMember)
EXPOSE_END
#undef EXPOSE_TYPE

class FPSCounter : public Engine::Graphic {
public:
	Engine & engine;

	double samplingDuration;
	double lastTime;
	int nbSamples;
	int lastFps;

	FPSCounter(Engine& e) : engine(e) {
		engine.overlayGraphic.AddChild(this);

		samplingDuration = 0.5;
		lastTime = engine.time;
		nbSamples = 0;
		lastFps = (int)(1.0 / engine.dtTarget);
	}

	void Draw() {

		++nbSamples;
		if (engine.time - lastTime > samplingDuration) {
			lastFps = (int)((double)nbSamples / samplingDuration);
			nbSamples = 0;
			lastTime = engine.time;
		}

		al_draw_textf(fetchDefaultFont(), al_map_rgb(255, 255, 255), 10, 10, 0, "FPS : %i", lastFps);
	}
};

int main(int nbarg, char ** args) {
	Engine engine;

	engine.graphics.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	engine.graphics.pointLights[0][0] = glm::vec4(5.0f, 0.0f, 1.0f, 3.0f);
	engine.graphics.pointLights[0][1] = glm::vec4(0.7f, 0.0f, 0.0f, 0.0f);
	engine.graphics.pointLights[1][0] = glm::vec4(0.0f, 1.0f, 0.75f, 1.5f);
	engine.graphics.pointLights[1][1] = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);

	if (engine.Init()) {
		al_init_primitives_addon();
		al_init_ttf_addon();

		TestCamera camera(engine);
		EngineLevel lvl(engine,"niveau.lvl");
		FPSCounter fc(engine);

		while (engine.OneLoop()) {}
	}
}

/*
#include <iostream>
#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\display.h>
#include <allegro5\events.h>
#include "JackamikazGUI.h"

int main(int nbarg, char ** args)
{

	al_init();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	ALLEGRO_DISPLAY* display = al_create_display(640, 480);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_install_mouse();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_install_keyboard();
	al_register_event_source(queue, al_get_keyboard_event_source());

	ALLEGRO_EVENT event;

	jmg::WallPaper wp(al_map_rgb(200, 200, 200));
	jmg::Window win(200,250,"Salut les gens");
	jmg::Text text(u"Salut je teste ma vie\ngenre lol ������������ ouais trop bien tavu ouech genre vazi quoi");
	jmg::MoveableRectangle mr(200,350);
	mr.addChild(&text);
	wp.addChild(&mr);

	win.mColor.g = 0;
	win.mOutline = 1;
	win.mRelx = 300;
	win.mRely = 150;

	text.mRelx = 10;
	text.mRely = 10;
	text.mWidth = mr.mWidth - text.mRelx * 2;

	win.setParent(&wp, false);


	bool quitApp = false;
	while (!quitApp)
	{
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			quitApp = true;
		}
		else {
			if (event.type == ALLEGRO_EVENT_KEY_UP
			 && event.keyboard.keycode == ALLEGRO_KEY_F1) {
				win.open();
			}
			wp.baseHandleEvent(event);
			wp.baseDraw();
			al_flip_display();
		}
	}

	al_destroy_font(jmg::fetchDefaultFont());
	al_destroy_display(display);

	al_shutdown_primitives_addon();

	return 0;
}
*/
