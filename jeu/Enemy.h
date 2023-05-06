#pragma once

#include "Game.h"
#include "Player.h"

class Enemy : public UpdateTask, public IDrawTask, public MovingEntity {
public:
    Vector3 position;
    const Vector3& target;
    const SlashTest& slash;
    int hit;

    Enemy(const VoxelMap& v, Vector3 spawnposition, const Vector3& target, const SlashTest& slash);

    void Do();

    void Draw();
};