#pragma once

#include "Voxel.h"

class MovingEntity {
public:
    const VoxelMap& voxels;
    Vector3& position;
    Vector3 velocity;
    bool grounded;
    float capsuleradius;
    Vector3 capsuletip;

    MovingEntity(const VoxelMap& v, Vector3& position);

    void ApplyGravity();
    void ApplyVelocity();
    void AdjustToCollisions();
};