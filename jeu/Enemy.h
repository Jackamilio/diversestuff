#pragma once

#include "Game.h"
#include "Player.h"

class Enemy : public UpdateTask, public IDrawTask, public MovingEntity {
public:
    Vector3 position;
    Vector3 patrolCenter;
    const Vector3& target;
    const SlashTest& slash;
    float hit;
    int state;

    Enemy(const VoxelMap& v, Vector3 spawnposition, const Vector3& target, const SlashTest& slash);

    void Do();

    void Draw();

    bool IsSeeingPlayer() const;
};