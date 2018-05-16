#ifndef ___EDITOR_CAMERA_H___
#define ___EDITOR_CAMERA_H___

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class EditorCamera {
public:
	EditorCamera();
	EditorCamera(const EditorCamera& cam);
	void operator=(const EditorCamera& cam);

	void SetFocusPoint(float x, float y, float z);
	void SetUp(float x, float y, float z);

	void SetAngles(float hor, float ver);
	void SetDistance(float dist, float min = 0.1f);
	void SetDistance(float dist, float min, float max);

	void CalcMatrix(glm::mat4x4& matrix);
	glm::vec3 GetPosition();

	glm::vec3 focuspoint;
	glm::vec3 up;

	float horAngle;
	float verAngle;
	float distance;
};

#endif//___EDITOR_CAMERA_H___