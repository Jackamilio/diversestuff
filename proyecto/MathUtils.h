#ifndef ___MATH_UTILS_H___
#define ___MATH_UTILS_H___

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class LocRotScale {
public:
	glm::vec3 loc;
	glm::quat rot;
	glm::vec3 scale;

	LocRotScale();
	glm::mat4 ToMat4() const;
};

bool linePlaneIntersection(const glm::vec3& plane_center, const glm::vec3& plane_normal, const glm::vec3& line_origin, const glm::vec3& line_direction, bool lineasray, glm::vec3* res = 0);
bool pointInTriangle(const glm::vec3& point, const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3);
bool lineTriangleIntersection(const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3, const glm::vec3& line_origin, const glm::vec3& line_direction, bool lineasray, glm::vec3* res = 0);

#endif//___MATH_UTILS_H___