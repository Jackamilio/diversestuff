#include "Engine.h"
#include <allegro5/allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "JackamikazGUI.h"
#include "LevelData.h"
#include "Dump.h"
#include "Editorcamera.h"
#include "Exposing.h"

class TestCamera : public Engine::Input, public Engine::Update {
public:
	Engine & engine;
	EditorCamera camera;
	//btRigidBody* trackbody;

	bool mleft;

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
		glEndList();

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
	}

	~EngineLevel() {
		//delete body;
		//delete motion;
		//delete shape;
		//delete levelMesh;
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4());
		model->Draw();
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
	}

	IM_AN_EXPOSER
};

#define EXPOSE_TYPE ExposingTest
EXPOSE_START
EXPOSE(_bool)
EXPOSE(_char)
EXPOSE(_uchar)
EXPOSE(_short)
EXPOSE(_ushort)
EXPOSE(_member)
EXPOSE(_int)
EXPOSE(_uint)
EXPOSE(_float)
EXPOSE(_double)
EXPOSE(_string)
EXPOSE_IC(_vector)
EXPOSE_IC(_vectorMember)
EXPOSE_END
#undef EXPOSE_TYPE

void callbackAdd(void* exptest) {
	ExposingTest& et = *(ExposingTest*)exptest;
	if (et._vector.empty()) {
		et._vector.push_back(0);
	}
	else {
		int val = (*et._vector.rbegin());
		et._vector.push_back(val + 1);
	}
}
void callbackRem(void* exptest) {
	ExposingTest& et = *(ExposingTest*)exptest;
	if (!et._vector.empty()) {
		et._vector.erase(et._vector.begin());
	}
}

class JmGui : public Engine::Graphic, public Engine::Input {
public:
	Engine & engine;

	jmg::Root root;
	jmg::Window win;
	jmg::ShowHide sh;
	std::vector<jmg::Label*> labels;

	jmg::Cropper crp;
	jmg::Label crplbl;
	jmg::Button crpbtn;
	jmg::MoveableDrawableRectangle crpmvb;

	ExposingTest test;

	jmg::Button add;
	jmg::Button rem;

	JmGui(Engine& e)
		: engine(e)
		, win(200, 250, "Salut les gens")
		, crp(200,200)
		, crplbl("Salut")
		, crpmvb(60,20)
		, add(30,30)
		, rem(30,30)
	{
		engine.overlayGraphic.AddChild(this);
		engine.inputRoot.AddChild(this,true);

		win.mOutline = 1;
		win.mRelx = 300;
		win.mRely = 150;
		win.setContext(&root, false);

		labels.push_back(new jmg::Label("Show Hide Test"));
		labels.push_back(new jmg::Label("Hidden 1"));
		labels.push_back(new jmg::Label("Hidden 2"));
		labels.push_back(new jmg::Label("Hidden 3"));
		labels.push_back(new jmg::Label("Below 1"));
		labels.push_back(new jmg::Label("Below 2"));

		win.addChild(&sh,10,10);
		//sh.mNbObjAlwaysShow = 2;
		sh.setAsAutoAddRef(10,-3,5);
		unsigned int i = 0;
		labels[i++]->autoAdd();
		sh.autoAddShift(5, 0);
		for (; i < 4; ++i) { labels[i]->autoAdd(); }
		sh.autoAddShift(-5, 0);
		for (; i < 6; ++i) { labels[i]->autoAdd(&win); }
		sh.hide();

		root.addChild(&crp, 100, 100);
		crp.addChild(&crplbl, 100, 100);
		crp.addChild(&crpbtn, 50, 50);
		crp.addChild(&crpmvb);

		add.mCallback = callbackAdd;
		add.mCallbackArgs = (void*)&test;
		rem.mCallback = callbackRem;
		rem.mCallbackArgs = (void*)&test;
	}

	~JmGui() {
		for (unsigned int i = 0; i < labels.size(); ++i) {
			delete labels[i];
		}
		labels.clear();
	}

	void Draw() {
		test._char++;
		test._member._otherShort--;
		test._vectorMember[2]._otherShort++;
		root.requestRedraw();
		root.baseDraw();
	}

	bool Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_KEY_UP) {
			if (event.keyboard.keycode == ALLEGRO_KEY_F1) {
				win.open();
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_F2) {
				jmg::Window* watcher = Exposing::createWatcherFor(test);
				watcher->setContext(&root, true);
				watcher->addChild(&add, 0, 0);
				watcher->addChild(&rem, 30, 0);
			}
		}
		return root.baseHandleEvent(event);
	}
};

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

		al_draw_textf(jmg::fetchDefaultFont(), al_map_rgb(255, 255, 255), 10, 10, 0, "FPS : %i", lastFps);
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
		JmGui gui(engine);

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
	jmg::Text text(u"Salut je teste ma vie\ngenre lol ‡‡‡‡‡‡ÈÈÈÈÈÈ ouais trop bien tavu ouech genre vazi quoi");
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
