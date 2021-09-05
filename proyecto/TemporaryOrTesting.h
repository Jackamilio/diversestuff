#ifndef __TEMPORARY_OR_TESTING_H__
#define __TEMPORARY_OR_TESTING_H__
#include "Engine.h"
#include "LevelData.h"

class EngineLevel : public Engine::Graphic {
public:
	Engine& engine;
	LevelData lvldt;
	const Model* model;

	//btTriangleMesh* levelMesh;
	//btBvhTriangleMeshShape* shape;
	//btMotionState* motion;
	//btRigidBody* body;

	EngineLevel(Engine& e, const char* level) : engine(e) {
		lvldt.Load(level);
		model = &engine.graphics.models.Get(&lvldt);
		//DrawLevelData(lvldt, engine.graphics.textures, true);

		/*btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));
		levelMesh = ConstructLevelCollision(lvldt);
		shape = new btBvhTriangleMeshShape(levelMesh, true);
		motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, shape);
		body = new btRigidBody(info);
		engine.physics->addRigidBody(body);*/

		engine.mainGraphic.AddChildToProgram(this, "test.pgr");
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
	}
};

class TestModel : public Engine::Update, public Engine::DoubleGraphic {
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

#endif __TEMPORARY_OR_TESTING_H__