#ifndef __TEMPORARY_OR_TESTING_H__
#define __TEMPORARY_OR_TESTING_H__
#include "Engine.h"
#include "MapData.h"
#include "Dump.h"

class EngineMap : public Engine::Graphic {
public:
	OTN(EngineMap)
	MapData mapdt;

	btTriangleMesh* mapMesh;
	btBvhTriangleMeshShape* shape;
	btMotionState* motion;
	btRigidBody* body;

	EngineMap(const char* map) :
		mapMesh(nullptr),
		shape(nullptr),
		motion(nullptr),
		body(nullptr)
	{
		mapdt.Load(map);

		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));
		mapMesh = ConstructMapCollision(mapdt);
		shape = new btBvhTriangleMeshShape(mapMesh, true);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, shape);
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);

		engine.mainGraphic.AddChildToProgram(this, "test.pgr");
	}

	~EngineMap() {
		engine.physics->removeRigidBody(body);
		delete body;
		delete motion;
		delete shape;
		delete mapMesh;
	}

	void Draw() {
		const Model* model = &engine.graphics.models.Get(&mapdt);
		if (model) {
			engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4(1.0));
			model->Draw();
		}
	}
};

class TestCamera : public Engine::Update {
public:
	OTN(TestCamera)
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

class TestModel : public Engine::Update, public Engine::DoubleGraphic {
public:
	OTN(TestModel);
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
		//model.GetPose("Marche", (float)engine.time, wpose, true);
		//model.GetPose("Cours", (float)engine.time, wpose2, true);
		//model.MixPoses(wpose, wpose2, glm::clamp((float)glm::sin(engine.time * 0.5f) + 0.5f, 0.0f, 1.0f));
		//model.FinalizePose(wpose, fpose);

		model.GetPose(animName, (float)engine.time, wpose, true);
		model.FinalizePose(wpose, fpose);
	}

	void Draw() {
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", glm::mat4(0.13));
		model.Draw(fpose);
	}

	void SecondDraw() {
		model.DrawPose(wpose);
	}
};

class TestCharacter : public Engine::Dynamic, public Engine::Update, public Engine::DoubleGraphic {
public:
	OTN(TestCharacter);
	IMPLEMENT_EXPOSE {
		EXPOSE_VALUE(speed);
		EXPOSE_VALUE(modelscale);
		EXPOSE_VALUE(walkrunblend);
	}
	float speed = 5.0f;
	float modelscale = 0.13f;
	float walkrunblend = 0.0f;

	btCapsuleShape* shape;
	btMotionState* motion;
	btRigidBody* body;

	bool grounded;
	glm::vec3 groundContact;

	const Model& model;
	Model::WorkingPose wpose;
	Model::WorkingPose wpose2;
	Model::FinalPose fpose;

	TestCharacter(const char* modelfile) :
		model(engine.graphics.models.Get(modelfile))
	{
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
		engine.debugGraphic.AddChild(GetSecondGraphic());
		engine.mainGraphic.AddChildToProgram(this, "test.pgr");
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
		float scl = al_key_down(&Engine::Input::keyboardState, ALLEGRO_KEY_SPACE) ? speed : 0.0f;
		EditorCamera& camera = engine.Access<TestCamera*>()->camera;
		body->setLinearVelocity(btVector3(cos(camera.horAngle) * scl, -sin(camera.horAngle) * scl, grounded ? 0.0f : body->getLinearVelocity().z()));
		body->setGravity(btVector3(0.0f, 0.0f, grounded ? 0.0f : -10.0f));
		body->activate();
	}

	void Step() {
		grounded = false;

		float looptime = fmodf((float)engine.time, 3.0f) / 3.0f;
		model.GetPose("walk", looptime * model.GetAnimDuration("walk"), wpose);
		model.GetPose("run", looptime * model.GetAnimDuration("run"), wpose2);
		model.MixPoses(wpose, wpose2, walkrunblend);
		model.FinalizePose(wpose, fpose);
		//engine.graphics.pointLights[2][0] = glm::vec4(pos.x(), pos.y(), pos.z(), 1.5f + sinf(engine.time));
		//engine.graphics.pointLights[2][1] = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);
	}

	void Draw() {
		const btVector3& pos = body->getCenterOfMassTransform().getOrigin();

		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, glm::vec3(pos.x(), pos.y(), pos.z() - shape->getHalfHeight() - shape->getRadius()));
		mat = glm::scale(mat, glm::vec3(modelscale, modelscale, modelscale));
		engine.graphics.programs.GetCurrent()->SetUniform("trWorld", mat);
		model.Draw(fpose);
	}

	void SecondDraw() {
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