#include "editorcamera.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <glm/gtc/matrix_transform.hpp>

EditorCamera::EditorCamera() : focuspoint(0,0,0), up(0,0,1), horAngle(0), verAngle(0), distance(1)
{
}

EditorCamera::EditorCamera(const EditorCamera & cam) : focuspoint(cam.focuspoint), up(cam.up), horAngle(cam.horAngle), verAngle(cam.verAngle), distance(cam.distance)
{
}

void EditorCamera::operator=(const EditorCamera & cam)
{
	focuspoint = cam.focuspoint;
	up = cam.up;
	horAngle = cam.horAngle;
	verAngle = cam.verAngle;
	distance = cam.distance;
}

void EditorCamera::SetFocusPoint(float x, float y, float z)
{
	focuspoint.x = x;
	focuspoint.y = y;
	focuspoint.z = z;
}

void EditorCamera::SetUp(float x, float y, float z)
{
	up.x = x;
	up.y = y;
	up.z = z;
}

void EditorCamera::SetAngles(float hor, float ver)
{
	horAngle = hor;
	verAngle = ver;

	if (verAngle >= glm::half_pi<float>()) {
		verAngle = glm::half_pi<float>() * 0.999f;
	}
	else if (verAngle <= -glm::half_pi<float>()) {
		verAngle = -glm::half_pi<float>() * 0.999f;
	}
}

void EditorCamera::SetDistance(float dist, float min)
{
	distance = dist;
	if (distance < min) {
		distance = min;
	}
}

void EditorCamera::SetDistance(float dist, float min, float max)
{
	SetDistance(dist, min);
	if (distance > max) {
		distance = max;
	}
}

void EditorCamera::CalcMatrix(glm::mat4x4& matrix)
{
	matrix = glm::lookAt(GetPosition(), focuspoint, up);
}

glm::vec3 EditorCamera::GetPosition()
{
	glm::vec3 vec(0, 0, 0);
	up = glm::normalize(up);
	float px = abs(up.x);
	float py = abs(up.y);
	float pz = abs(up.z);
	if (px > py && px > pz) {
		vec.y = up.y < 0 ? -1.0f : 1.0f;
	}
	else if (py > pz) {
		vec.z = up.z < 0 ? -1.0f : 1.0f;
	}
	else {
		vec.x = up.x < 0 ? -1.0f : 1.0f;
	}
	glm::vec3 front = glm::normalize(glm::cross(up, vec));
	glm::vec3 side = glm::normalize(glm::cross(front, up));

	float ch = cos(-horAngle);
	float sh = sin(-horAngle);
	float cv = cos(-verAngle);
	float sv = sin(-verAngle);

	float ax = ch * cv * distance;
	float ay = sh * cv * distance;
	float az = sv * distance;

	glm::vec3 from = focuspoint;
	from -= side * ax + front * ay + up * az;

	return from;
}
