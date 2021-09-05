#include "MathUtils.h"
#include <glm/gtc/matrix_transform.hpp>

bool linePlaneIntersection(const glm::vec3& plane_center, const glm::vec3& plane_normal, const glm::vec3& line_origin, const glm::vec3& line_direction, bool lineasray, glm::vec3* res)
{
	//plane_normal = glm::normalize(plane_normal);
	//line_direction = glm::normalize(line_direction);

	float denom = glm::dot(plane_normal, line_direction);
	if (abs(denom) > 0.0001f)
	{
		float t = glm::dot((plane_center - line_origin), plane_normal) / denom;
		glm::vec3 intersect = line_origin + line_direction * t;
		if (lineasray && glm::dot(intersect - line_origin, line_direction) < 0.0f) {
			return false;
		}
		if (res) {
			*res = intersect;
		}
		return true;
	}
	return false;
}

// thank you here! http://www.blackpawn.com/texts/pointinpoly/
// are P1 and P2 in the same side compared to [AB] ?
bool sameSide(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 A, const glm::vec3 B) {
	glm::vec3 cp1 = glm::cross(B - A, p1 - A);
	glm::vec3 cp2 = glm::cross(B - A, p2 - A);
	return glm::dot(cp1, cp2) >= 0.0f;
}

bool pointInTriangle(const glm::vec3& point, const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3)
{
	return sameSide(point, t1, t2, t3) && sameSide(point, t2, t1, t3) && sameSide(point, t3, t1, t2);
	/* This code is totally irrelevant ... but I keep the record of this error
	glm::vec3 ab = t2 - t1;
	glm::vec3 bc = t3 - t2;
	glm::vec3 ca = t1 - t3;

	glm::vec3 ap = point - t1;
	glm::vec3 bp = point - t2;
	glm::vec3 cp = point - t3;

	float d1 = glm::dot(ab, ap);
	float d2 = glm::dot(bc, bp);
	float d3 = glm::dot(ca, cp);

	return (d1 >= 0 && d2 >= 0 && d3 >= 0);// || (d1 <= 0 && d2 <= 0 && d3 <= 0);*/
}

bool lineTriangleIntersection(const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3, const glm::vec3& line_origin, const glm::vec3& line_direction, bool lineasray, glm::vec3* res)
{
	glm::vec3 plane_normal = glm::normalize(glm::cross(t2 - t1, t3 - t1));
	glm::vec3 point;
	if (linePlaneIntersection(t1, plane_normal, line_origin, line_direction, lineasray, &point)) {
		if (pointInTriangle(point, t1, t2, t3)) {
			if (res) {
				*res = point;
			}
			return true;
		}
	}
	return false;
}

LocRotScale::LocRotScale() :
	loc(0.0f, 0.0f, 0.0f),
	rot(0.0f, 0.0f, 0.0f, 1.0f),
	scale(1.0f, 1.0f, 1.0f)
{
}

glm::mat4 LocRotScale::ToMat4() const
{
	glm::mat4 tr = glm::mat4(1.0f);
	tr = glm::translate(tr, loc);

	glm::mat4 mrot = glm::mat4_cast(rot);
	tr = tr * mrot;

	tr = glm::scale(tr, scale);
	return tr;
}
