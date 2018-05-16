#include "AppTest.h"
#include <stdio.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include "dump.h"
#include <allegro5/allegro_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"
#include "editorcamera.h"
#include <btBulletDynamicsCommon.h>
#include "LevelData.h"
#include "TextureManager.h"
#include <FL/Fl.h>

void* fltk_thread(ALLEGRO_THREAD* t, void* arg) {
	
	Gui::GuiContext* gui = Gui::Load();
	Fl::lock();

	bool cont = true;
	while (cont) {
		Fl::wait(1e20);
		void* msg = Fl::thread_message();
		if (msg) {
			cont = Gui::HandleMessage(gui, msg);
		}
	}
	Gui::Unload(gui);

	return 0;
}

AppTest::AppTest() :
	fltkThread(NULL),
	init_success(false)
{
	fltkThread = al_create_thread(fltk_thread, 0);

	al_start_thread(fltkThread);

	engine = new Engine();
	init_success = engine->Init();

	run();
}

AppTest::~AppTest()
{
	if (init_success) {
		void** ret = 0;
		Gui::Send(Gui::Message::quit);
		al_join_thread(fltkThread, ret);
		al_destroy_thread(fltkThread);
	}

	if (engine) {
		delete engine;
		engine = 0;
	}
}

class EngineLevel : public Engine::Graphic {
public:
	Engine& engine;
	LevelData lvldt;
	const Model* model;

	btTriangleMesh* levelMesh;
	btBvhTriangleMeshShape* shape;
	btMotionState* motion;
	btRigidBody* body;

	EngineLevel(Engine& e, const char* level) : engine(e) {
		lvldt.Load(level);
		model = &engine.graphics.models.Get(&lvldt);
		DrawLevelData(lvldt, engine.graphics.textures, true);
		glEndList();

		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));
		levelMesh = ConstructLevelCollision(lvldt);
		shape = new btBvhTriangleMeshShape(levelMesh, true);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, shape);
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);

		engine.mainGraphic.AddChildForProgram(this, "test.pgr");
	}

	~EngineLevel() {
		delete body;
		delete motion;
		delete shape;
		delete levelMesh;
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4());
		model->Draw();
	}
};

class FPSCounter : public Engine::Graphic {
public:
	Engine& engine;

	double samplingDuration;
	double lastTime;
	int nbSamples;
	int lastFps;

	FPSCounter(Engine& e) : engine(e) {
		engine.Get<ALLEGRO_FONT*>() = al_load_bitmap_font("a4_font.tga");
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

		al_draw_textf(engine.Get<ALLEGRO_FONT*>(), al_map_rgb(255, 255, 255), 10, 10, 0, "FPS : %i", lastFps);
	}
};

class TestCamera : public Engine::Input, public Engine::Update {
public:
	Engine& engine;
	EditorCamera camera;
	btRigidBody* trackbody;

	bool mleft;

	TestCamera(Engine& e) : engine(e) {
		mleft = false;
		trackbody = 0;

		engine.inputRoot.AddChild(this);
		engine.updateRoot.AddChild(this);

		camera.distance = 10.0f;
		camera.SetFocusPoint(0, 0, 0.5f);
		camera.SetUp(0, 0, 1);

		engine.graphics.proj = glm::perspective(glm::radians(40.0f), 640.0f / 480.0f, 0.5f, 50.0f);
	}

	void Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
			mleft = true;
		}
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
			mleft = false;
		}
		if (event.type == ALLEGRO_EVENT_MOUSE_AXES && mleft) {
			camera.horAngle += (float)event.mouse.dx * 0.01f;
			camera.verAngle += (float)event.mouse.dy * 0.01f;
		}
	}

	void Step() {

		if (trackbody) {
			btTransform tr = trackbody->getCenterOfMassTransform();
			camera.SetFocusPoint(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
		}

		camera.CalcMatrix(engine.graphics.view);
	}
};

class TestCube : public Engine::Input, public Engine::Dynamic, public Engine::Update, public Engine::DoubleGraphic {
public:
	Engine& engine;

	btCapsuleShape* shape;
	btMotionState* motion;
	btRigidBody* body;

	bool space;
	bool grounded;
	glm::vec3 groundContact;

	TestCube(Engine& e) :engine(e) {
		space = false;
		grounded = false;

		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 5));
		t.setRotation(btQuaternion(0.0f, glm::half_pi<float>(), 0.0f));
		//shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
		shape = new btCapsuleShape(0.25f, 0.7f);
		btVector3 inertia;
		//shape->calculateLocalInertia(1.0f, inertia);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(1.0f, motion, shape, inertia);
		info.m_friction = 0.0f;
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);
		ReactToCollisionFrom(*body);

		engine.inputRoot.AddChild(this);
		engine.dynamicRoot.AddChild(this);
		engine.updateRoot.AddChild(this);
		engine.debugGraphic.AddChild(this);
		engine.overlayGraphic.AddChild(GetSecondGraphic());
	}

	void Event(ALLEGRO_EVENT& event) {
		if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
				space = true;
			}
		}

		if (event.type == ALLEGRO_EVENT_KEY_UP) {
			if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
				space = false;
			}
		}
	}

	void Collision(Dynamic* other, btPersistentManifold& manifold) {
		for (int i = 0; i < manifold.getNumContacts(); ++i) {
			btManifoldPoint& pt = manifold.getContactPoint(i);
			if (pt.getPositionWorldOnA().z() < body->getCenterOfMassPosition().z() - shape->getHalfHeight() - shape->getRadius() * 0.5f) {
				grounded = true;
				groundContact = btToVec(pt.getPositionWorldOnA());
				break;
			}
		}
	}

	void Tick() {
		float scl = space ? 5.0 : 0.0f;
		EditorCamera& camera = engine.Get<TestCamera*>()->camera;
		body->setLinearVelocity(btVector3(cos(camera.horAngle) * scl, -sin(camera.horAngle) * scl, grounded ? 0.0f : body->getLinearVelocity().z()));
		body->setGravity(btVector3(0.0f, 0.0f, grounded ? 0.0f : -10.0f));
		body->activate();
	}

	void Step() {
		grounded = false;

		const btVector3& pos = body->getCenterOfMassTransform().getOrigin();

		//engine.graphics.pointLights[2][0] = glm::vec4(pos.x(), pos.y(), pos.z(), 1.5f + sinf(engine.time));
		//engine.graphics.pointLights[2][1] = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);
	}

	void Draw() {
		btTransform tr = body->getCenterOfMassTransform();
		glm::mat4 mat;
		tr.getOpenGLMatrix(&mat[0].x);

		glPushMatrix();
		glMultMatrixf(&mat[0].x);
		DrawGlWireCapsule(shape->getRadius(), shape->getHalfHeight() * 2.0f);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(groundContact.x, groundContact.y, groundContact.z);
		glColor3ub(255, 0, 0);
		DrawGlWireCube(-0.05f, 0.05f);
		glPopMatrix();
	}

	void SecondDraw() {
		al_draw_text(engine.Get<ALLEGRO_FONT*>(), al_map_rgb(255, 255, 255), 10, 30, 0, "Second graphic success!");
	}

	~TestCube() {
		delete body;
		delete motion;
		delete shape;
	}
};

class TestModel : public Engine::Update, public Engine::DoubleGraphic{
public:
	Engine& engine;
	const Model& model;
	Model::WorkingPose wpose;
	Model::WorkingPose wpose2;
	Model::FinalPose fpose;
	const char* animName;
	
	TestModel(Engine& e, const char* modelfile, const char* anim)
		: engine(e)
		, model(e.graphics.models.Get(modelfile))
		, animName(anim)
	{
		engine.updateRoot.AddChild(this);
		engine.mainGraphic.AddChildForProgram(this, "test.pgr");
		//engine.debugGraphic.AddChild(GetSecondGraphic());
	}

	void Step() {
		model.GetPose("Marche", (float)engine.time, wpose, true);
		model.GetPose("Cours", (float)engine.time, wpose2, true);
		model.MixPoses(wpose, wpose2, glm::clamp((float)glm::sin(engine.time*0.5f) + 0.5f, 0.0f, 1.0f));
		model.FinalizePose(wpose, fpose);
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4());
		model.Draw(fpose);
	}

	void SecondDraw() {
		model.DrawPose(wpose);
	}
};

void AppTest::run()
{
	if (!init_success) return;

	EngineLevel el(*engine, "niveau.lvl");
	FPSCounter fc(*engine);
	TestCube tc(*engine);
	TestCamera cam(*engine);
	TestModel tm(*engine, "personnage.fbx", "Cours");
	cam.trackbody = tc.body;

	engine->Get<TestCamera*>() = &cam;

	engine->graphics.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	engine->graphics.pointLights[0][0] = glm::vec4(5.0f, 0.0f, 1.0f, 3.0f);
	engine->graphics.pointLights[0][1] = glm::vec4(0.7f, 0.0f, 0.0f, 0.0f);
	engine->graphics.pointLights[1][0] = glm::vec4(0.0f, 1.0f, 0.75f, 1.5f);
	engine->graphics.pointLights[1][1] = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);

	while (engine->OneLoop()) {}

	delete engine;
	engine = 0;
}
