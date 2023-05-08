#pragma once

#include "raylib.h"
#include "raylibext.h"
#include <vector>
#include <list>

Vector3 ProjectPointToLine(const Vector3& point, const Vector3& lineA, const Vector3& lineB);
Vector3 ProjectPointToSegment(const Vector3& point, const Segment& segment);
float DistanceToSegmentSqr(const Vector3& point, const Segment& segment);
//Vector3 ProjectPointToBox(const Vector3& point, const BoundingBox& box);
//float DistanceToBoxSqr(const Vector3& point, const BoundingBox& box);
float DistanceToPlaneSigned(const Vector3& point, const Vector4& plane); // plane.xyz must be normalized, plane.w is d component
Vector3 ProjectPointToPlane(const Vector3& point, const Vector4& plane); // plane.xyz must be normalized, plane.w is d component
Vector3 ClosestSegmentPointToPlane(const Segment& segment, const Vector4& plane);
Vector3 ClosestPlanePointToSegment(const Vector4& plane, const Segment& segment);
bool IsPointInPolygon(const Vector3& point, const std::vector<Vector3>& polygon); // the polygon must be convex and all points are assumed in the same plane
//std::vector<Vector3> CalculateSegmentBoxIntersections(const Segment& segment, const BoundingBox& box);
Vector3 ClosestSegmentPointToSegment(const Segment& segmentA, const Segment& segmentB);
Vector3 ClosestBoxPointToSegment(const Box& box, const Segment& segment);

struct Shape;
struct CollisionPoint {
	Vector3 position{ 0.0f, 0.0f, 0.0f };
//	Vector3 normal{ 0.0f, 0.0f, 0.0f };
//	float penetration{ 0.0f };
	const Shape* shape{ nullptr };
};
struct Shape {
	enum Type { RAY = 0, PLANE, BOX, SPHERE, CAPSULE, SHAPEAMOUNT };
	union {
		Vector3 position; // all other shapes MUST have their first component represent the position
		Box box;
		Ray ray;
		Vector4 plane;
		Sphere sphere;
		Capsule capsule;
		Matrix matrix; // here for the 16 float size
	};
	Vector3 boundingBoxHalfSizes;
	Type type;
	unsigned int mask;
	unsigned int wantedMask;
	void* user;
	std::vector<CollisionPoint> points;
	// bvh address
};

typedef std::list<Shape*> ShapeList;
typedef ShapeList::iterator ShapeLocation;

class Collisions {
private:
	ShapeList shapes;
	ShapeList seekers;

public:
	ShapeLocation AddShape(Shape& shape); // must be called after the shape was properly initialized
	void RemoveShape(ShapeLocation& loc);
	void ShapePositionUpdated(ShapeLocation& loc);
	void ShapeVolumeUpdated(ShapeLocation& loc);
	inline bool IsLocationValid(const ShapeLocation& loc) const {
		return loc != shapes.end();
	}
	inline ShapeLocation InvalidLocation() {
		return shapes.end();
	}

	Collisions();
	void Update();
};