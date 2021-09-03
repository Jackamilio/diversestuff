#ifndef ___EDITOR_CAMERA_H___
#define ___EDITOR_CAMERA_H___

#include "Engine.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class EditorCamera {
public:
	EditorCamera();
	EditorCamera(const EditorCamera& cam);
	void operator=(const EditorCamera& cam);

	void SetFocusPoint(float x, float y, float z);
	void SetUp(float x, float y, float z);

	void SetAngles(float hor, float ver, bool relative = false);
	void SetDistance(float dist, float min = 0.1f);
	void SetDistance(float dist, float min, float max);

	void CalcMatrix(glm::mat4x4& matrix);
	glm::vec3 GetPosition();

	//fine tuned input, if you want more condition for when those happen
	//otherwise just use the DefaultInput class
	bool Drag(ALLEGRO_EVENT& event, int mousebutton = 1);
	bool Zoom(ALLEGRO_EVENT& event, float scale = 1.0f, float* powerDist = nullptr, float powerIncrement = 0.2f, float maxpowerdist = FLT_MAX);

	glm::vec3 focuspoint;
	glm::vec3 up;

	float horAngle;
	float verAngle;
	float distance;

	class DefaultInput;
};

class EditorCamera::DefaultInput : public EditorCamera, public Engine::Input {
public:
	float powerDist;

	DefaultInput(float powerDist = 1.0f);
	bool Event(ALLEGRO_EVENT& event);
	inline bool Zoom(ALLEGRO_EVENT& event, float scale = 1.0f, float powerIncrement = 0.2f, float maxpowerdist = FLT_MAX) {
		return EditorCamera::Zoom(event, scale, &powerDist, powerIncrement, maxpowerdist);
	}
};

#endif//___EDITOR_CAMERA_H___