#pragma once

#include "Voxel.h"

class MovingEntity : CollisionChecker {
public:
    //const VoxelMap& voxels;
    //Vector3& position;
    Vector3 velocity;
    bool grounded;
    //float capsuleradius;
    //Vector3 capsuletip;

    ShapeLocation shapeloc;

    MovingEntity();

    void CheckCollision();
    virtual void PostCollision() = 0;

    void ApplyGravity();
    void ApplyVelocity();
    //void AdjustToCollisions();
};