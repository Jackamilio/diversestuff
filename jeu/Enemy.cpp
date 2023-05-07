#include "Enemy.h"

enum {
    PATROL = 0,
    PURSUE,
    PREATTACK,
    ATTACK,
    POSTATTACK,
    HIT,
    DIZZY
};

const char* strStates[] = {
    "patrol",
    "pursue",
    "preattack",
    "attack",
    "postattack",
    "hit",
    "dizzy"
};

Enemy::Enemy(const VoxelMap& v, Vector3 spawnposition, const Vector3& target, const SlashTest& slash) :
    UpdateTask(PRIO_NORMAL),
    MovingEntity(v, position),
    position{ spawnposition },
    patrolCenter{ position },
    target(target),
    slash(slash),
    statetimer(0.0f),
    state(PATROL)
{
    //capsuletip = Vector3Zero();
}

void Enemy::Do() {
    switch (state) {
    case PATROL:
        CircleTarget(patrolCenter, game.settings.critter.patrolCenterDistance, game.settings.critter.patrolSpeed);

        // check if the player is in sight
        if (IsSeeingPlayer()) {
            state = PURSUE;
            statetimer = GetRandomValue(game.settings.critterAttack.minInBetweenDuration, game.settings.critterAttack.maxInBetweenDuration);
        }
        break;
    case PURSUE:
        CircleTarget(target, 2.0f, game.settings.critter.chaseSpeed);

        // back to patrol if the player is out of sight
        if (!IsSeeingPlayer()) {
            state = PATROL;
        }
        else {
            TimerBasedNextState(PREATTACK, game.settings.critterAttack.windingDuration);
        }

        break;
    case PREATTACK:
        if (TimerBasedNextState(ATTACK, game.settings.critterAttack.attackDuration)) {
            // set the velocity just once when changing state
            MoveTowardsPlayer(game.settings.critterAttack.attackSpeed);
            velocity.y += 3.0f;
        }
        else {
            MoveTowardsPlayer(game.settings.critterAttack.windingSpeed);
        }
        break;
    case ATTACK:
        TimerBasedNextState(POSTATTACK, game.settings.critterAttack.postAttackPauseDuration);
        break;
    case HIT:
        MoveTowardsPlayer(-1.0f);
        TimerBasedNextState(PATROL);
        break;
    case DIZZY:
    case POSTATTACK:
        velocity.x = 0.0f;
        velocity.z = 0.0f;
        TimerBasedNextState(PATROL);
        break;
    }

    // hit test
    float dist = capsuleradius + 1.0f;
    if (state != HIT && slash.IsSlashing() && Vector3DistanceSqr(position, slash.GetPosition()) < dist * dist) {
        statetimer = 0.33f;
        velocity.y += 3.0f;
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
    ImGui::Text("State %s %f", strStates[state], statetimer);
}

void Enemy::Draw() {
    Color c = SKYBLUE;
    switch (state) {
    case HIT:
        c = RandomColor();
        break;
    case PREATTACK:
    case ATTACK:
        c = RED;
        break;
    }
    DrawSphere(position, capsuleradius, c);
}

bool Enemy::IsSeeingPlayer() const
{
    const float sd = game.settings.critter.sightDistance;
    const float disttoplayersqr = Vector3DistanceSqr(position, target);
    if (disttoplayersqr < sd * sd) {
        Ray metoplayer{ position, Vector3Normalize(target - position) };
        RayCollision col = GetRayCollisionVoxelMap(metoplayer, voxels);
        return !col.hit || disttoplayersqr < col.distance * col.distance;
    }
    return false;
}

void Enemy::MoveTowardsPlayer(float distance)
{
    Vector3 targeting = target - position;
    targeting.y = 0.0f;
    targeting = Vector3Normalize(targeting) * distance;

    velocity.x = targeting.x;
    velocity.z = targeting.z;
}

bool Enemy::TimerBasedNextState(int newState, float newTimer)
{
    statetimer -= game.deltaTime;
    if (statetimer <= 0.0f) {
        state = newState;
        statetimer = newTimer;
        return true;
    }
    return false;
}

void Enemy::CircleTarget(const Vector3& tgt, float distance, float speed)
{
    Vector3 targettome = position - tgt;
    targettome.y = 0.0f;
    const float targetdistance = Vector3Length(targettome);
    // if we're at spawn point, move away
    if (targetdistance < speed) {
        velocity.x = speed;
        velocity.z = 0.0f;
    }
    // if we're at the patrol circle, continue along the circle
    else if (std::abs(targetdistance - distance) < speed * game.deltaTime * 1.2f)
    {
        const float angularspeed = speed * game.deltaTime / targetdistance;
        const float currentangle = atan2f(targettome.z, targettome.x);
        const float newangle = currentangle + angularspeed;
        targettome.x = cosf(newangle) * targetdistance;
        targettome.z = sinf(newangle) * targetdistance;
        Vector3 newpos = tgt + targettome;
        velocity.x = (newpos.x - position.x) / game.deltaTime;
        velocity.z = (newpos.z - position.z) / game.deltaTime;
    }
    // other wise go to the circle
    else {
        targettome *= speed / targetdistance;
        if (targetdistance > distance) {
            targettome = -targettome;
        }
        velocity.x = targettome.x;
        velocity.z = targettome.z;
    }
}

