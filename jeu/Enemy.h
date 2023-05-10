#pragma once

#include "Game.h"
#include "Player.h"

class Enemy : public UpdateTask, public IDrawTask, public MovingEntity {
public:
    Vector3 patrolCenter;
    const Vector3& target;
    const SlashTest& slash;
    float statetimer;
    int state;

    Enemy(const VoxelMap& v, Vector3 spawnposition, const Vector3& target, const SlashTest& slash);
    ~Enemy();

    void PostCollision();

    void Do();

    void Draw();

    bool IsSeeingPlayer() const;
    void MoveTowardsPlayer(float distance);

    bool TimerBasedNextState(int newstate, float newtimer = FLT_MAX);
    void CircleTarget(const Vector3& target, float distance, float speed);
};