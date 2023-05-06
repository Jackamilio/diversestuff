#include "Enemy.h"

enum {
    PATROL,
    PURSUE,
    PREATTACK,
    ATTACK
};

Enemy::Enemy(const VoxelMap& v, Vector3 spawnposition, const Vector3& target, const SlashTest& slash) :
    UpdateTask(PRIO_NORMAL),
    MovingEntity(v, position),
    position{ spawnposition },
    target(target),
    slash(slash),
    hit(0)
{
    //capsuletip = Vector3Zero();
}

void Enemy::Do() {

    // move towards target
    Vector3 targeting = target - position;
    targeting.y = 0.0f;
    targeting = Vector3Normalize(targeting) * 0.02f;

    velocity.x = targeting.x;
    velocity.z = targeting.z;

    float dist = capsuleradius + 1.0f;
    if (!hit && slash.IsSlashing() && Vector3DistanceSqr(position, slash.GetPosition()) < dist * dist) {
        hit = 20;
        velocity.y += 0.05f;
    }
    else if (hit > 0) {
        Vector3 recoil = velocity * (hit * -0.2f);
        recoil.y = 0.0f;
        velocity += recoil;
        --hit;
    }

    ApplyGravity();
    ApplyVelocity();
    AdjustToCollisions();

    game.AddDrawTask(*this, DRAW_3D_SHADER);
    game.AddShadowCaster(*this);

    //Vector3 sp = slash.GetPosition();
    //ImGui::InputFloat3("Enemy position", &position.x);
    //ImGui::InputFloat3("Slash position", &sp.x);
    //ImGui::LabelText("Distance to slash", "%f", Vector3Distance(sp, position));
}

void Enemy::Draw() {
    DrawSphere(position, capsuleradius, hit ? RandomColor() : SKYBLUE);
}
