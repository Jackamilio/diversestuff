#include "MovingEntity.h"
#include "collision.h"

MovingEntity::MovingEntity(const VoxelMap& v, Vector3& position) :
    voxels(v),
    position(position),
    grounded(false),
    velocity{ 0.0f },
    capsuleradius(0.5f),
    capsuletip({ 0.0f, 1.0f, 0.0f })
{

}

void MovingEntity::ApplyGravity() {
    const float gravity = 0.005f;

    if (!grounded) {
        velocity.y -= gravity;
    }
}

void MovingEntity::ApplyVelocity() {
    position = position + velocity;// Vector3{ 0.0f, velocity.y, 0.0f };
}

void MovingEntity::AdjustToCollisions() {
    Vector3 capsuleB = position + capsuletip;
    Vector3 capsulecenter = position + capsuletip * 0.5f;
    const float capsuleradiussqr = capsuleradius * capsuleradius;

    Vector3 upvec = { 0.0f, 1.0f, 0.0f };

    std::multimap<float, Vector3> potentialcollisions;
    potentialcollisions.insert({ 0.0f, ClosestPlanePointToSegment({0.0f, 1.0f, 0.0f, 0.0f}, {position, capsuleB}) });

    for (const auto& voxel : voxels) {
        Vector3 close = ClosestBoxPointToSegment(voxel.first.ToBoundingBox(), { position, capsuleB });
        potentialcollisions.insert({ Vector3DistanceSqr(capsulecenter, close), close });
    }

    grounded = false;

    for (const auto& elem : potentialcollisions) {
        const Vector3& col = elem.second;
        Vector3 close = ProjectPointToSegment(col, { position, capsuleB });
        if (Vector3DistanceSqr(close, col) <= capsuleradiussqr) {

            Vector3 colnormal = close - col;
            float coldist = Vector3Length(colnormal);
            if (coldist < 0.00001f) {
                colnormal = Vector3Normalize(capsuleB - position);
            }
            else {
                colnormal = colnormal / coldist;
            }
            float dotnormal = Vector3DotProduct(upvec, colnormal);
            if (dotnormal > 0.0f) {
                grounded = true;
            }

            if (velocity.y * dotnormal < 0.0f) {
                velocity.y = 0.0f;
            }

            Vector3 movement = colnormal * (capsuleradius - coldist);
            position = position + movement;
            capsuleB = capsuleB + movement;
        }
    }
}