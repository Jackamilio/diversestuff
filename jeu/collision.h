#pragma once

#include "raylib.h"
#include "raylibext.h"
#include <vector>
#include <list>
#include <unordered_set>

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
		Ray ray;
		Vector4 plane;
		Box box;
		Sphere sphere;
		Capsule capsule;
		Matrix matrix; // here for the 16 float size
	};
	BoundingBox boundingBox;
	Type type;
	unsigned int mask;
	unsigned int wantedMask;
	//void* user;
	std::vector<CollisionPoint> points;
	// bvh address
};

typedef std::list<Shape> ShapeList;
typedef ShapeList::const_iterator ShapeLocation;

class Collisions {
private:
	class BoundaryVolumeHierarchy {
	public:
		struct Node {
			BoundingBox boundingBox;
			ShapeLocation shapeloc;
			Node* parent;
			Node* nodes[2];

			inline bool IsLeafLess() {
				return !nodes[0] && !nodes[1];
			}

			inline bool IsRoot() {
				return !parent;
			}

			inline bool IsLeaf() {
				return game.collisions.IsLocationValid(shapeloc);
			}

			Node();
			Node(ShapeLocation loc);
		};

	private:
		Node root;

		bool InsertNodeRecursive(Node& currentNode, ShapeLocation loc);
	public:
		BoundaryVolumeHierarchy();

		void Insert(ShapeLocation loc);
	};

	ShapeList shapes;
	std::unordered_set<Shape*> seekers;
	std::unordered_set<Shape*> updatedshapes;

	// https://stackoverflow.com/questions/765148/how-to-remove-constness-of-const-iterator
	inline Shape& AccessShape(ShapeLocation loc) {
		return *shapes.erase(loc, loc);
	}

	ShapeLocation AddShape(Shape& shape, unsigned int mask, unsigned int wantedMask);
	void UpdateShapeVolume(Shape& shape);
public:
	ShapeLocation AddRay(const Ray& ray, unsigned int mask, unsigned int wantedMask);
	ShapeLocation AddPlane(const Vector4& plane, unsigned int mask, unsigned int wantedMask);
	ShapeLocation AddBox(const Box& box, unsigned int mask, unsigned int wantedMask);
	ShapeLocation AddSphere(const Sphere& sphere, unsigned int mask, unsigned int wantedMask);
	ShapeLocation AddCapsule(const Capsule& capsule, unsigned int mask, unsigned int wantedMask);

	void RemoveShape(ShapeLocation& loc);

	void UpdateShapePosition(const Vector3& newposition, ShapeLocation& loc);

	//void ShapePositionUpdated(ShapeLocation& loc);
	void UpdateRay(const Ray& ray, ShapeLocation& loc);
	void UpdatePlane(const Vector4& ray, ShapeLocation& loc);
	void UpdateBox(const Box& ray, ShapeLocation& loc);
	void UpdateSphere(const Sphere& ray, ShapeLocation& loc);
	void UpdateCapsule(const Capsule& ray, ShapeLocation& loc);

	inline bool IsLocationValid(const ShapeLocation& loc) const {
		return loc != shapes.end();
	}
	inline ShapeLocation InvalidLocation() {
		return shapes.end();
	}

	Collisions();
	void Update();
};