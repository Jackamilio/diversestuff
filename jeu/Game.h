#pragma once
#include <map>
#include <vector>
#include "raylib.h"
#include "Settings.h"

class IUpdateTask {
public:
	virtual void Do() = 0;
};

class IDrawTask {
public:
	virtual void Draw() = 0;
};

typedef enum {
	PRIO_HIGH = 0,
	PRIO_NORMAL,
	PRIO_LATE
} Priority;

typedef enum {
	DRAW_3D_LIGHTS = -1, // negative DrawPriority are called before BeginDrawing, they're responsible for all their graphic process
	DRAW_3D_UNIFORMS = 0,
	DRAW_3D_SHADER,
	DRAW_3D_NOSHADER,
	DRAW_2D_FIRST,
	DRAW_2D_NORMAL,
	DRAW_2D_LATE
} DrawPriority;

typedef std::vector<std::reference_wrapper<IDrawTask>> ShadowCasters;

struct Input {
	struct Button {
		bool previousframe = false;
		bool currentframe = false;
		inline bool IsDown() { return currentframe; }
		inline bool IsUp() { return !currentframe; }
		inline bool IsPressed() { return !previousframe && currentframe; }
		inline bool IsReleased() { return previousframe && !currentframe; }
		inline void SetCurrentFrame(bool down) { previousframe = currentframe; currentframe = down; }
	};
	Vector2 movement{ 0.0f, 0.0f };
	Vector2 view{ 0.0f, 0.0f };
	Button jump;
	Button attack;
	Button placeblock;
	Button removeblock;
};

class Game
{
private:
	std::multimap<int, IUpdateTask&> update; // user is responsible for inserting and removing
	std::multimap<int, IDrawTask&> draw; // is emptied each frame
	ShadowCasters shadowcasters; // free to add yourself here, lights will handle it, also emptied each frame
public:
	inline void AddUpdateTask(IUpdateTask& task, int priority) {
		update.insert({ priority, task });
	}
	inline void AddDrawTask(IDrawTask& task, int priority) {
		draw.insert({ priority, task });
	}
	inline void AddShadowCaster(IDrawTask& task) {
		shadowcasters.push_back(task);
	}
	inline const ShadowCasters& GetShadowCasters() const {
		return shadowcasters;
	}

	void RemoveUpdateTask(IUpdateTask& task, int priority);

	Camera camera;
	Shader mainshader;
	Settings settings;
	Input input;
	float deltaTime;

	Game();

	void Init();
	void Loop();
	void Quit();
};

class UpdateTask : public IUpdateTask {
private:
	int priority;

public:
	UpdateTask(int prio);
	~UpdateTask();
};

class DrawTask : public UpdateTask, public IDrawTask {
private:
	int drawpriority;

public:
	DrawTask(int prio);
	void Do();
};

extern Game game;