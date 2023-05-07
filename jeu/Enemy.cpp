#include "Enemy.h"

enum {
    PATROL = 0,
    PURSUE,
    PREATTACK,
    ATTACK,
    HIT
};

Enemy::Enemy(const VoxelMap& v, Vector3 spawnposition, const Vector3& target, const SlashTest& slash) :
    UpdateTask(PRIO_NORMAL),
    MovingEntity(v, position),
    position{ spawnposition },
    patrolCenter{ position },
    target(target),
    slash(slash),
    hit(0),
    state(PATROL)
{
    //capsuletip = Vector3Zero();
}

void Enemy::Do() {
    switch (state) {
    case PATROL:
    {
        Vector3 patroling = position - patrolCenter;
        patroling.y = 0.0f;
        float patroldistance = Vector3Length(patroling);
        float thisframespeed = game.settings.critter.patrolSpeed * game.deltaTime;
        // if we're at spawn point, move away
        if (patroldistance < thisframespeed) {
            velocity.x = thisframespeed;
            velocity.z = 0.0f;
        }
        // if we're at the patrol circle, continue along the circle
        else if (std::abs(patroldistance - game.settings.critter.patrolCenterDistance) < thisframespeed)
        {
            const float pcd = game.settings.critter.patrolCenterDistance;
            const float angularspeed = thisframespeed / pcd;
            const float currentangle = atan2f(patroling.z, patroling.x);
            const float newangle = currentangle + angularspeed;
            patroling.x = cosf(newangle) * pcd;
            patroling.z = sinf(newangle) * pcd;
            Vector3 newpos = patrolCenter + patroling;
            velocity.x = newpos.x - position.x;
            velocity.z = newpos.z - position.z;
        }
        // other wise go to the circle
        else {
            patroling *= thisframespeed / patroldistance;
            if (patroldistance > game.settings.critter.patrolCenterDistance) {
                patroling = -patroling;
            }
            velocity.x = patroling.x;
            velocity.z = patroling.z;
        }

        // check if the player is in sight
        if (IsSeeingPlayer()) {
            state = PURSUE;
        }
    }
        break;
    case PURSUE:
    {
        // move towards target
        Vector3 targeting = target - position;
        targeting.y = 0.0f;
        targeting = Vector3Normalize(targeting) * (game.settings.critter.chaseSpeed * game.deltaTime);

        velocity.x = targeting.x;
        velocity.z = targeting.z;

        // back to patrol if the player is out of sight
        if (!IsSeeingPlayer()) {
            state = PATROL;
        }
    }
        break;
    case PREATTACK:

        break;
    case ATTACK:

        break;
    case HIT:
    {
        Vector3 recoil = Vector3Normalize(position - target);
        recoil.y = 0.0f;
        recoil *= game.deltaTime;

        velocity.x = recoil.x;
        velocity.z = recoil.z;

        hit -= game.deltaTime;
        if (hit <= 0.0f) {
            state = PURSUE;
        }
    }
        break;
    }

    // hit test
    float dist = capsuleradius + 1.0f;
    if (state != HIT && slash.IsSlashing() && Vector3DistanceSqr(position, slash.GetPosition()) < dist * dist) {
        hit = 0.33f;
        velocity.y += 0.05f;
        state = HIT;
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
    DrawSphere(position, capsuleradius, (state == HIT) ? RandomColor() : SKYBLUE);
}

bool Enemy::IsSeeingPlayer() const
{
    const float sd = game.settings.critter.sightDistance;
    if (Vector3DistanceSqr(position, target) < sd * sd) {
        Ray metoplayer{ position, Vector3Normalize(target - position) };
        RayCollision col = GetRayCollisionVoxelMap(metoplayer, voxels);
        return !col.hit;
    }
    return false;
}
