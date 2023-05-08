#include "MovingEntity.h"
#include "collision.h"

MovingEntity::MovingEntity(const VoxelMap& v, Shape& shape) :
    CollisionChecker(0),
    voxels(v),
    //position(position),
    grounded(false),
    velocity{ 0.0f },
    shape(shape)
    //capsuleradius(0.5f),
    //capsuletip({ 0.0f, 1.0f, 0.0f })
{

}

void MovingEntity::CheckCollision()
{
    grounded = false;

    if (shape.points.empty()) return;

    std::multimap<float, Vector3> orderedpoints;
    const Vector3 capsulecenter = shape.capsule.GetCenter();

    for (auto& point : shape.points) {
        orderedpoints.insert({Vector3DistanceSqr(point.position, capsulecenter), point.position});
    }

    for (const auto& elem : orderedpoints) {
        //Vector3 col = elem.position + elem.normal * elem.penetration;
        const Vector3& col = elem.second;
        Vector3 close = ProjectPointToSegment(col, shape.capsule.GetSegment());
        if (Vector3DistanceSqr(close, col) <= shape.capsule.radiussqr()) {

            Vector3 colnormal = close - col;
            float coldist = Vector3Length(colnormal);
            if (coldist < 0.00001f) {
                colnormal = Vector3Normalize(shape.capsule.offset);
            }
            else {
                colnormal = colnormal / coldist;
            }
            float dotnormal = Vector3DotProduct({ 0.0f, 1.0f, 0.0f }, colnormal);
            if (dotnormal > 0.0f) {
                grounded = true;
            }

            if (velocity.y * dotnormal < 0.0f) {
                velocity.y = 0.0f;
            }

            shape.position += colnormal * (shape.capsule.radius - coldist);
        }
    }

    PostCollision();
}

void MovingEntity::ApplyGravity() {
    const float gravity = 0.3f;

    if (!grounded) {
        velocity.y -= gravity;
    }
}

void MovingEntity::ApplyVelocity() {
    shape.position += velocity * game.deltaTime;
}

/*void MovingEntity::AdjustToCollisions() {
    Vector3& position = shape.position;
    Vector3 capsuleB = shape.capsule.GetTip();
    Vector3 capsulecenter = shape.capsule.GetCenter();
    const float capsuleradiussqr = shape.capsule.radiussqr();

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

            Vector3 movement = colnormal * (shape.capsule.radius - coldist);
            position = position + movement;
            capsuleB = capsuleB + movement;
        }
    }
}*/
