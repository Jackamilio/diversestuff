#pragma once

#include "raylib.h"
#include "raylibext.h"
#include <vector>

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