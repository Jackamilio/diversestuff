#ifndef __TEMPORARY_OR_TESTING_H__
#define __TEMPORARY_OR_TESTING_H__
#include "Engine.h"
#include "MapData.h"
#include "Dump.h"

class EngineLevel : public Engine::Graphic {
public:
	MapData lvldt;
	const Model* model;

	btTriangleMesh* levelMesh;
	btBvhTriangleMeshShape* shape;
	btMotionState* motion;
	btRigidBody* body;

	EngineLevel(const char* level) :
		levelMesh(nullptr),
		shape(nullptr),
		motion(nullptr),
		body(nullptr)
	{
		lvldt.Load(level);
		model = &engine.graphics.models.Get(&lvldt);

		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));
		levelMesh = ConstructLevelCollision(lvldt);
		shape = new btBvhTriangleMeshShape(levelMesh, true);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, shape);
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);

		engine.mainGraphic.AddChildToProgram(this, "test.pgr");
	}

	~EngineLevel() {
		engine.physics->removeRigidBody(body);
		delete body;
		delete motion;
		delete shape;
		delete levelMesh;
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4(1.0));
		model->Draw();
	}
};

class TestModel : public Engine::Update, public Engine::DoubleGraphic {
public:
	const Model& model;
	Model::WorkingPose wpose;
	Model::WorkingPose wpose2;
	Model::FinalPose fpose;
	const char* animName;

	TestModel(const char* modelfile, const char* anim) :
		model(engine.graphics.models.Get(modelfile)),
		animName(anim)
	{
		engine.updateRoot.AddChild(this);
		engine.mainGraphic.AddChildToProgram(this, "test.pgr");
		//engine.debugGraphic.AddChild(GetSecondGraphic());
	}

	void Step() {
		model.GetPose("Marche", (float)engine.time, wpose, true);
		model.GetPose("Cours", (float)engine.time, wpose2, true);
		model.MixPoses(wpose, wpose2, glm::clamp((float)glm::sin(engine.time * 0.5f) + 0.5f, 0.0f, 1.0f));
		model.FinalizePose(wpose, fpose);
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4(1.0));
		model.Draw(fpose);
	}

	void SecondDraw() {
		model.DrawPose(wpose);
	}
};

class TestCamera : public Engine::Update {
public:
	EditorCamera::DefaultInput camera;
	btRigidBody* trackbody;

	TestCamera() :
		trackbody(nullptr)
	{
		engine.inputRoot.AddChild(&camera);
		engine.updateRoot.AddChild(this);

		camera.distance = 10.0f;
		camera.SetFocusPoint(0, 0, 0.5f);
		camera.SetUp(0, 0, 1);

		engine.graphics.proj = glm::perspective(glm::radians(40.0f), 1280.0f / 720.0f, 0.5f, 50.0f);
	}

	void Step() {

		if (trackbody) {
			btTransform tr = trackbody->getCenterOfMassTransform();
			camera.SetFocusPoint(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
		}

		camera.CalcMatrix(engine.graphics.view);
	}
};

class TestCharacter : public Engine::Dynamic, public Engine::Update, public Engine::Graphic {
public:
	btCapsuleShape* shape;
	btMotionState* motion;
	btRigidBody* body;

	bool grounded;
	glm::vec3 groundContact;

	TestCharacter() {
		grounded = false;

		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 5));
		t.setRotation(btQuaternion(0.0f, glm::half_pi<float>(), 0.0f));
		shape = new btCapsuleShape(0.25f, 0.7f);
		btVector3 inertia;
		//shape->calculateLocalInertia(1.0f, inertia);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(1.0f, motion, shape, inertia);
		info.m_friction = 0.0f;
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);
		ReactToCollisionFrom(*body);

		engine.dynamicRoot.AddChild(this);
		engine.updateRoot.AddChild(this);
		engine.debugGraphic.AddChild(this);
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
		float scl = al_key_down(&Engine::Input::keyboardState, ALLEGRO_KEY_SPACE) ? 5.0 : 0.0f;
		EditorCamera& camera = engine.Access<TestCamera*>()->camera;
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
		glm::mat4 mat(1.0);
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

	~TestCharacter() {
		engine.physics->removeRigidBody(body);
		delete body;
		delete motion;
		delete shape;
	}
};

#endif __TEMPORARY_OR_TESTING_H__