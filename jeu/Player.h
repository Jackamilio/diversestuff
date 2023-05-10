#pragma once

#include "Game.h"
#include "MovingEntity.h"

class SlashTest : public UpdateTask, public IDrawTask {
private:
    Model model;
    std::vector<Texture2D> texanim;
    int animtick;
public:
    SlashTest();
    ~SlashTest();

    void Do();

    bool IsSlashing() const;

    Vector3 GetPosition() const;

    void Draw();

    void Launch(const Vector3& from, float direction, float tilt);
};

class Player : public UpdateTask, public IDrawTask, public MovingEntity {
public:
    float direction;
    SlashTest slash;

    Player(const VoxelMap& v);
    ~Player();

    void Do();
    void PostCollision();

    void Draw();
};