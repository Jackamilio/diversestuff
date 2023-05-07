#pragma once

#include "raylib.h"
#include "raylibext.h"
#include <vector>
#include <forward_list>

Vector3 ProjectPointToLine(const Vector3& point, const Vector3& lineA, const Vector3& lineB);
Vector3 ProjectPointToSegment(const Vector3& point, const Segment& segment);
float DistanceToSegmentSqr(const Vector3& point, const Segment& segment);
Vector3 ProjectPointToBox(const Vector3& point, const BoundingBox& box);
float DistanceToBoxSqr(const Vector3& point, const BoundingBox& box);
float DistanceToPlaneSigned(const Vector3& point, const Vector4& plane); // plane.xyz must be normalized, plane.w is d component
Vector3 ProjectPointToPlane(const Vector3& point, const Vector4& plane); // plane.xyz must be normalized, plane.w is d component
Vector3 ClosestSegmentPointToPlane(const Segment& segment, const Vector4& plane);
Vector3 ClosestPlanePointToSegment(const Vector4& plane, const Segment& segment);
bool IsPointInPolygon(const Vector3& point, const std::vector<Vector3>& polygon); // the polygon must be convex and all points are assumed in the same plane
std::vector<Vector3> CalculateSegmentBoxIntersections(const Segment& segment, const BoundingBox& box);
Vector3 ClosestSegmentPointToSegment(const Segment& segmentA, const Segment& segmentB);
Vector3 ClosestBoxPointToSegment(const BoundingBox& box, const Segment& segment);

class Collisions {
public:
	struct Shape;
	struct CollisionPoint {
	public:
		Vector3 position{ 0.0f, 0.0f, 0.0f };
	private:
		Vector3 normal{ 0.0f, 0.0f, 0.0f };
		bool isNormalUnit{ false };
	public:
		const Vector3& GetNormal() {
			if (!isNormalUnit) {
				normal *= 1.0f / Vector3Length(normal);
				isNormalUnit = true;
			}
			return normal;
		}
		const Shape& shape;
	};
	struct Shape {
		enum Type { BOX, RAY, PLANE, SPHERE, CAPSULE };
		union {
			BoundingBox box;
			Ray ray;
			Vector4 plane;
			Sphere sphere;
			Capsule capsule;
			Matrix matrix;
		};
		Type type;
		unsigned int mask;
		unsigned int wantedMask;
		void* user;
		std::vector<CollisionPoint> points;
		// octree address
	};

	typedef std::forward_list<Shape> ShapeList;
	typedef ShapeList::iterator ShapeLocation;

private:
	ShapeList shapes;

public:

	Collisions();
	void Update();
};