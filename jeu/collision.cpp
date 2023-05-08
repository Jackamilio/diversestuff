#include "collision.h"
#include "raylibext.h"
#include <algorithm>
#include <cmath>

// projection of vector A onto B
inline Vector3 ProjectVector(const Vector3& A, const Vector3& B) {
	return B * (Vector3DotProduct(B, A) / Vector3LengthSqr(B));
}

Vector3 ProjectPointToLine(const Vector3& point, const Vector3& lineA, const Vector3& lineB) {
	Vector3 AB = lineA - lineB;
	Vector3 Atopoint = point - lineA;
	float projscalar = Vector3DotProduct(AB, Atopoint) / Vector3LengthSqr(AB);
	return lineA + AB * projscalar;
}

Vector3 ProjectPointToSegment(const Vector3& point, const Segment& segment)
{
	Vector3 AB = segment.pointB - segment.pointA;
	Vector3 Atopoint = point - segment.pointA;
	float projscalar = Vector3DotProduct(AB, Atopoint) / Vector3LengthSqr(AB);
	return segment.pointA + AB * std::clamp(projscalar, 0.0f, 1.0f);
}

float DistanceToSegmentSqr(const Vector3& point, const Segment& segment)
{
	return Vector3DistanceSqr(point,ProjectPointToSegment(point, segment));
}

Vector3 ProjectPointToBox(const Vector3& point, const BoundingBox& box)
{
	return Vector3{
		std::clamp(point.x, box.min.x, box.max.x),
		std::clamp(point.y, box.min.y, box.max.y),
		std::clamp(point.z, box.min.z, box.max.z)};
}

float DistanceToBoxSqr(const Vector3& point, const BoundingBox& box)
{
	return Vector3DistanceSqr(point, ProjectPointToBox(point, box));
}

inline const Vector3& Vector3Component(const Vector4& vec4) {
	return *((Vector3*)&vec4);
}

float DistanceToPlaneSigned(const Vector3& point, const Vector4& plane)
{
	return Vector3DotProduct(point, Vector3Component(plane)) - plane.w;
}

Vector3 ProjectPointToPlane(const Vector3& point, const Vector4& plane)
{
	return point - Vector3Component(plane) * DistanceToPlaneSigned(point, plane);
}

Vector3 ClosestSegmentPointToPlane(const Segment& segment, const Vector4& plane)
{
	float distA = DistanceToPlaneSigned(segment.pointA, plane);
	float distB = DistanceToPlaneSigned(segment.pointB, plane);

	return segment.Lerp(std::clamp(distA / (distA - distB), 0.0f, 1.0f));
}

Vector3 ClosestPlanePointToSegment(const Vector4& plane, const Segment& segment)
{
	float distA = DistanceToPlaneSigned(segment.pointA, plane);
	float distB = DistanceToPlaneSigned(segment.pointB, plane);

	bool Aiscloser = std::abs(distA) < std::abs(distB);

	return (Aiscloser ? segment.pointA : segment.pointB) - Vector3Component(plane) * (Aiscloser ? distA : distB);
}

bool IsPointInPolygon(const Vector3& point, const std::vector<Vector3>& polygon)
{
	if (polygon.size() < 3) {
		return false; // useless if the polygon hasn't at least three vertices (not a polygon!)
	}

	const Vector3& Z = polygon[polygon.size() - 1]; // fetching the last vertex
	Vector3 ZA = polygon[0] - Z; // handling the last vector first therefore no need to care for the index loop
	Vector3 planenormal = Vector3CrossProduct(polygon[1] - polygon[0], ZA); // the plane normal used for all tests, doesn't need to be unit

	Vector3 tangent = Vector3CrossProduct(planenormal, ZA); // first tangent
	float firstdot = Vector3DotProduct(tangent, point - Z); // first side test for the point, all other dots need to have the same sign

	for (unsigned int i = 0; i < polygon.size() - 1; i++) {
		tangent = Vector3CrossProduct(planenormal, polygon[i+1] - polygon[i]); // current tangent
		float curdot = Vector3DotProduct(tangent, point - polygon[i]); // current dot
		if (firstdot * curdot < 0.0f) { // if the dots don't have the same sign, the point is outside the polygon
			return false;
		}
	}
	return true;
}

inline bool IsInSquare(float x, float y, float squareleft, float squarebottom, float squareright, float squaretop) {
	return x >= squareleft && x < squareright && y >= squarebottom && y < squaretop;
}

#define COMPONENT_X 0
#define COMPONENT_Y 1
#define COMPONENT_Z 2

inline float GetVector3Component(const Vector3& vec, unsigned int component) {
	return *(&vec.x + component);
}

inline void SquareIntersection(
	const Vector3& A,
	const Vector3& B,
	const Vector3& AB,
	const Vector3& min,
	const Vector3& max,
	const Vector3& minormaxplane,
	std::vector<Vector3>& out,
	unsigned int width_component,
	unsigned int height_component,
	unsigned int depth_component) {
	// get A distance to the plane
	const float AdistToPlane = GetVector3Component(A, depth_component) - GetVector3Component(minormaxplane, depth_component);
	// get B distance to the plane
	const float BdistToPlane = GetVector3Component(B, depth_component) - GetVector3Component(minormaxplane, depth_component);
	if (AdistToPlane * BdistToPlane < 0.0f) { // if the sign is different, AB goes through the plane
		Vector3 I = A + AB * (AdistToPlane / (AdistToPlane - BdistToPlane)); // calculate the intersection in the plane
		if (IsInSquare(
			GetVector3Component(I, width_component), GetVector3Component(I, height_component),
			GetVector3Component(min, width_component), GetVector3Component(min, height_component),
			GetVector3Component(max, width_component), GetVector3Component(max, height_component))) { // if the intersection is inside the square
			out.push_back(I); // record it
		}
	}
}

std::vector<Vector3> CalculateSegmentBoxIntersections(const Segment& segment, const BoundingBox& box)
{
	const Vector3& A = segment.pointA;
	const Vector3& B = segment.pointB;
	const Vector3& min = box.min;
	const Vector3& max = box.max;

	std::vector<Vector3> out;

	Vector3 AB = B - A;

	SquareIntersection(A, B, AB, min, max, min, out, COMPONENT_X, COMPONENT_Y, COMPONENT_Z);// bottom square on xy plane
	SquareIntersection(A, B, AB, min, max, max, out, COMPONENT_X, COMPONENT_Y, COMPONENT_Z);// top square on xy plane
	SquareIntersection(A, B, AB, min, max, min, out, COMPONENT_Y, COMPONENT_Z, COMPONENT_X);// left square on yz plane
	SquareIntersection(A, B, AB, min, max, max, out, COMPONENT_Y, COMPONENT_Z, COMPONENT_X);// right square on yz plane
	SquareIntersection(A, B, AB, min, max, min, out, COMPONENT_X, COMPONENT_Z, COMPONENT_Y);// near square on xz plane
	SquareIntersection(A, B, AB, min, max, max, out, COMPONENT_X, COMPONENT_Z, COMPONENT_Y);// far square on xz plane

	return out;
	/*
	// bottom square on xy plane
	{
		const float AxyBBmin = A.z - min.z;
		const float BxyBBmin = B.z - min.z;
		if (AxyBBmin * BxyBBmin < 0.0f) {
			Vector3 I = A + AB * (AxyBBmin / (AxyBBmin - BxyBBmin));
			if (IsInSquare(I.x, I.y, min.x, min.y, max.x, max.y)) {
				out.push_back(I);
			}
		}
	}

	// top square on xy plane
	{
		const float AxyBBmax = A.z - max.z;
		const float BxyBBmax = B.z - max.z;
		if (AxyBBmax * BxyBBmax < 0.0f) {
			Vector3 I = A + AB * (AxyBBmax / (AxyBBmax - BxyBBmax));
			if (IsInSquare(I.x, I.y, min.x, min.y, max.x, max.y)) {
				out.push_back(I);
			}
		}
	}

	// left square on yz plane
	{
		const float AyzBBmin = A.x - min.x;
		const float ByzBBmin = B.x - min.x;
		if (AyzBBmin * ByzBBmin < 0.0f) {
			Vector3 I = A + AB * (AyzBBmin / (AyzBBmin - ByzBBmin));
			if (IsInSquare(I.y, I.z, min.y, min.z, max.y, max.z)) {
				out.push_back(I);
			}
		}
	}

	// right square on yz plane
	{
		const float AyzBBmax = A.x - max.x;
		const float ByzBBmax = B.x - max.x;
		if (AyzBBmax * ByzBBmax < 0.0f) {
			Vector3 I = A + AB * (AyzBBmax / (AyzBBmax - ByzBBmax));
			if (IsInSquare(I.y, I.z, min.y, min.z, max.y, max.z)) {
				out.push_back(I);
			}
		}
	}

	// near square on xz plane
	{
		const float AxzBBmin = A.y - min.y;
		const float BxzBBmin = B.y - min.y;
		if (AxzBBmin * BxzBBmin < 0.0f) {
			Vector3 I = A + AB * (AxzBBmin / (AxzBBmin - BxzBBmin));
			if (IsInSquare(I.x, I.z, min.x, min.z, max.x, max.z)) {
				out.push_back(I);
			}
		}
	}

	// far square on xz plane
	{
		const float AxzBBmax = A.y - max.y;
		const float BxzBBmax = B.y - max.y;
		if (AxzBBmax * BxzBBmax < 0.0f) {
			Vector3 I = A + AB * (AxzBBmax / (AxzBBmax - BxzBBmax));
			if (IsInSquare(I.x, I.z, min.x, min.z, max.x, max.z)) {
				out.push_back(I);
			}
		}
	}

	return out;*/
}

// using the technique of collapsing the CD segment to a point and projecting AB to the plane defined by C and CD
// thanks to this genius here : https://zalo.github.io/blog/closest-point-between-segments/
Vector3 ClosestSegmentPointToSegment(const Segment& segmentA, const Segment& segmentB)
{
	Vector3 CD = segmentB.pointB - segmentB.pointA;
	float invCDsqrmag = 1.0f / Vector3LengthSqr(CD);

	Vector3 inPlaneA = segmentA.pointA - (CD * (Vector3DotProduct(segmentA.pointA - segmentB.pointA, CD) * invCDsqrmag));
	Vector3 inPlaneB = segmentA.pointB - (CD * (Vector3DotProduct(segmentA.pointB - segmentB.pointA, CD) * invCDsqrmag));
	Vector3 inPlaneAB = inPlaneB - inPlaneA;

	float projscalar = Vector3DotProduct(inPlaneAB, segmentB.pointA - inPlaneA) / Vector3LengthSqr(inPlaneAB);
	return segmentA.Lerp(std::clamp(projscalar, 0.0f, 1.0f));
}

Vector3 ClosestBoxPointToSegment(const Box& box, const Segment& segment)
{
	const Vector3& A = segment.pointA;
	const Vector3& B = segment.pointB;
	const Vector3& min = box.position - box.halfSizes;
	const Vector3& max = box.position + box.halfSizes;

	Vector3 AB = B - A;

	float bestdist = FLT_MAX;
	Vector3 candidate{ 0.0f, 0.0f, 0.0f};
	Vector3 clp;

	// bottom square on xy
	Vector4 plane = {0.0f, 0.0f, 1.0f, min.z};

	clp = ClosestPlanePointToSegment(plane, segment);
	if (IsInSquare(clp.x, clp.y, min.x, min.y, max.x, max.y)) {
		// skipping the distance check, we know it's the first
		bestdist = DistanceToSegmentSqr(clp, segment);
		candidate = clp;
	}


	// top square on xy
	plane.w = max.z;

	clp = ClosestPlanePointToSegment(plane, segment);
	if (IsInSquare(clp.x, clp.y, min.x, min.y, max.x, max.y)) {
		float newdist = DistanceToSegmentSqr(clp, segment);
		if (newdist < bestdist) {
			bestdist = newdist;
			candidate = clp;
		}
	}

	// left square on yz
	plane.z = 0.0f;
	plane.x = 1.0f;
	plane.w = min.x;

	clp = ClosestPlanePointToSegment(plane, segment);
	if (IsInSquare(clp.y, clp.z, min.y, min.z, max.y, max.z)) {
		float newdist = DistanceToSegmentSqr(clp, segment);
		if (newdist < bestdist) {
			bestdist = newdist;
			candidate = clp;
		}
	}

	// right square on yz
	plane.w = max.x;

	clp = ClosestPlanePointToSegment(plane, segment);
	if (IsInSquare(clp.y, clp.z, min.y, min.z, max.y, max.z)) {
		float newdist = DistanceToSegmentSqr(clp, segment);
		if (newdist < bestdist) {
			bestdist = newdist;
			candidate = clp;
		}
	}

	// near square on xz
	plane.x = 0.0f;
	plane.y = 1.0f;
	plane.w = min.y;

	clp = ClosestPlanePointToSegment(plane, segment);
	if (IsInSquare(clp.x, clp.z, min.x, min.z, max.x, max.z)) {
		float newdist = DistanceToSegmentSqr(clp, segment);
		if (newdist < bestdist) {
			bestdist = newdist;
			candidate = clp;
		}
	}

	// far square on xz
	plane.w = max.y;

	clp = ClosestPlanePointToSegment(plane, segment);
	if (IsInSquare(clp.x, clp.z, min.x, min.z, max.x, max.z)) {
		float newdist = DistanceToSegmentSqr(clp, segment);
		if (newdist < bestdist) {
			bestdist = newdist;
			candidate = clp;
		}
	}

	//              H ------ G   <- max
	//            / |      / |
	// y z      E --+--- F   |
	// |/       |   |    |   |
	// *-x      |   D ---+-- C
	//          | /      | /
	// min ->   A ------ B

	Vector3 boxA = min;
	Vector3 boxB = { max.x, min.y, min.z };
	Vector3 boxC = { max.x, max.y, min.z };
	Vector3 boxD = { min.x, max.y, min.z };
	Vector3 boxE = { min.x, min.y, max.z };
	Vector3 boxF = { max.x, min.y, max.z };
	Vector3 boxG = max;
	Vector3 boxH = { min.x, max.y, max.z };

	float newdist;

	clp = ClosestSegmentPointToSegment({ boxA, boxB }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxB, boxC }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxC, boxD }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxD, boxA }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxE, boxF }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxF, boxG }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxG, boxH }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxH, boxE }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxA, boxE }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxB, boxF }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxC, boxG }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	clp = ClosestSegmentPointToSegment({ boxD, boxH }, segment);
	newdist = DistanceToSegmentSqr(clp, segment); if (newdist < bestdist) { bestdist = newdist; candidate = clp; }

	return candidate;
}

// All collision possibilities !
// Every Col_ function is responsible for adding points to the shape if applicable
// BOX, RAY, PLANE, SPHERE, CAPSULE

bool Col_RayRay(Shape&, Shape&, CollisionPoint*, CollisionPoint*) {
	return false;
}

bool Col_RayPlane(Shape& ray, Shape& plane, CollisionPoint* col1, CollisionPoint* col2) {
	Segment raysegment = { ray.ray.position , ray.ray.position + ray.ray.direction };

	float distA = DistanceToPlaneSigned(raysegment.pointA, plane.plane);
	float distB = DistanceToPlaneSigned(raysegment.pointB, plane.plane);

	float t = distA / (distA - distB);
	if (t >= 0.0f) {
		Vector3 colpoint = raysegment.Lerp(t);
		if (col1) {
			col1->position = colpoint;
		}
		if (col2) {
			col2->position = colpoint;
		}
		return true;
	}
	return false;
}

bool Col_PlaneCapsule(Shape& plane, Shape& capsule, CollisionPoint* col1, CollisionPoint* col2) {
	Segment capsuleSegment{ capsule.capsule.base, capsule.capsule.GetTip() };
	Vector3 planepoint = ClosestPlanePointToSegment(plane.plane, capsuleSegment);
	Vector3 segmentpoint = ProjectPointToSegment(planepoint, capsuleSegment);

	if (Vector3LengthSqr(planepoint - segmentpoint) <= capsule.capsule.radiussqr()) {
		if (col1) {
			col1->position = segmentpoint; // adjust to capsule surface?
		}
		if (col2) {
			col2->position = planepoint;
		}
		return true;
	}
	return false;
}
bool Col_BoxCapsule(Shape& box, Shape& capsule, CollisionPoint* col1, CollisionPoint* col2) {
	Segment capsuleSegment{ capsule.capsule.base, capsule.capsule.GetTip() };
	Vector3 boxpoint = ClosestBoxPointToSegment(box.box, capsuleSegment);
	Vector3 segmentpoint = ProjectPointToSegment(boxpoint, capsuleSegment);

	Vector3 colnormal = segmentpoint - boxpoint;
	float normallen = Vector3LengthSqr(colnormal);
	if (normallen <= capsule.capsule.radiussqr()) {
		/*if (col1 || col2) {
			if (normallen < 0.0000001f) {
				normallen = 0.0f;
				colnormal = Vector3Normalize(capsule.capsule.offset); // maybe there is a better choice
			}
			else {
				normallen = std::sqrtf(normallen);
				colnormal *= 1.0f / normallen;
			}

			const float penetration = capsule.capsule.radius - normallen;
			if (col1) {
				col1->position = boxpoint;
				col1->normal = -colnormal;
				col2->penetration = penetration;
			}
			if (col2) {
				col2->position = segmentpoint - colnormal * capsule.capsule.radius;
				col2->normal = colnormal;
				col2->penetration = penetration;
			}
		}*/
		if (col1) {
			col1->position = segmentpoint; // adjust to capsule surface?
		}
		if (col2) {
			col2->position = boxpoint;
		}
		return true;
	}
	return false;
}

bool Col_WrongOrder(Shape& box, Shape& ray, CollisionPoint* col1, CollisionPoint* col2) { return false; }
bool Col_Todo(Shape& box, Shape& ray, CollisionPoint* col1, CollisionPoint* col2) { return false; }

typedef bool (*Col_Function)(Shape&, Shape&, CollisionPoint*, CollisionPoint*);

Col_Function Col_FunctionMatrix[Shape::Type::SHAPEAMOUNT][Shape::Type::SHAPEAMOUNT] = {
	Col_RayRay,		Col_RayPlane,	Col_Todo,		Col_Todo,		Col_Todo,
	Col_WrongOrder,	Col_Todo,		Col_Todo,		Col_Todo,		Col_PlaneCapsule,
	Col_WrongOrder,	Col_WrongOrder,	Col_Todo,		Col_Todo,		Col_BoxCapsule,
	Col_WrongOrder,	Col_WrongOrder,	Col_WrongOrder,	Col_Todo,		Col_Todo,
	Col_WrongOrder,	Col_WrongOrder,	Col_WrongOrder,	Col_WrongOrder,	Col_Todo
};

void InfiniteHalfSizes(Shape& shape) {
	shape.boundingBoxHalfSizes.x = FLT_MAX;
	shape.boundingBoxHalfSizes.y = FLT_MAX;
	shape.boundingBoxHalfSizes.z = FLT_MAX;
}
void CalculateSphereHalfSizes(Shape& shape) {
	Box b = shape.sphere.GetBoundingBox();
	shape.boundingBoxHalfSizes = b.halfSizes;
}
void CalculateBoxHalfSizes(Shape& shape) {
	shape.boundingBoxHalfSizes = shape.box.halfSizes;
}
void CalculateCapsuleHalfSizes(Shape& shape) {
	Box b = shape.capsule.GetBoundingBox();
	shape.boundingBoxHalfSizes = b.halfSizes;
}

typedef void (*CalculateHalfSizeFunc)(Shape&);
CalculateHalfSizeFunc CalculateHalfSizeTable[Shape::Type::SHAPEAMOUNT] = {
	InfiniteHalfSizes, InfiniteHalfSizes, CalculateSphereHalfSizes, CalculateBoxHalfSizes, CalculateCapsuleHalfSizes
};

void Collisions::ShapePositionUpdated(ShapeLocation& loc)
{
	// todo BVH
}

void Collisions::ShapeVolumeUpdated(ShapeLocation& loc)
{
	Shape& s = **loc;
	CalculateHalfSizeTable[s.type](s);
}

ShapeLocation Collisions::AddShape(Shape& shape)
{
	if (shape.wantedMask) {
		seekers.push_back(&shape);
	}
	shapes.push_back(&shape);
	ShapeLocation end = shapes.end();
	--end;
	ShapeVolumeUpdated(end);
	return end;
}

void Collisions::RemoveShape(ShapeLocation& loc)
{
	if (IsLocationValid(loc)) {
		Shape* s = *loc;
		if (s->wantedMask) {
			seekers.remove(s);
		}
		shapes.erase(loc);
		loc = shapes.end();
	}
}

Collisions::Collisions()
{
}

void Collisions::Update()
{
	// first clear all previous collisions
	for (auto& shape : shapes) {
		shape->points.clear();
	}

	// now perform all collisions
	// BRUTE FORCE FOR NOW
	CollisionPoint cp1;
	CollisionPoint cp2;
	for (auto& leftShape : seekers) {
		for (auto& rightShape : shapes ) {
			if (leftShape != rightShape) {
				Box lb{ leftShape->position, leftShape->boundingBoxHalfSizes };
				Box rb{ rightShape->position, rightShape->boundingBoxHalfSizes };
				// small optimisation before implementing BVH
				if (lb.Intersects(rb)) {
					Shape* ls = leftShape;
					Shape* rs = rightShape;
					if (ls->type > rs->type) {
						std::swap(ls, rs);
					}

					CollisionPoint* wantcp1 = nullptr;
					CollisionPoint* wantcp2 = nullptr;
					if (ls->wantedMask | rs->mask) {
						wantcp1 = &cp1;
					}
					if (rs->wantedMask | ls->mask) {
						wantcp2 = &cp2;
					}

					if (wantcp1 || wantcp2) {
						if (Col_FunctionMatrix[ls->type][rs->type](*ls, *rs, wantcp1, wantcp2)) {
							if (wantcp1) {
								cp1.shape = rs;
								ls->points.push_back(cp1);
							}
							if (wantcp2) {
								cp2.shape = ls;
								rs->points.push_back(cp2);
							}
						}
					}
				}
			}
		}
	}
}
