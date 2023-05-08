#include "Player.h"
#include "rcamera.h"
#include "CollisionMask.h"
#include <string>

SlashTest::SlashTest() : UpdateTask(PRIO_NORMAL) {
    model = LoadModel("slash.obj");

    std::string file = "Blue Slash Wide/File1.png";
    while (FileExists(file.c_str())) {
        texanim.push_back(LoadTexture(file.c_str()));
        file[20] += 1;
    }

    //SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, texanim[0]);
    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    animtick = 0;
}

SlashTest::~SlashTest() {
    for (auto& tex : texanim) {
        UnloadTexture(tex);
    }
}

void SlashTest::Do() {

    int curframe = animtick / game.settings.slashFrameDuration;
    int newframe = ++animtick / game.settings.slashFrameDuration;
    if (newframe < texanim.size()) {
        if (curframe != newframe) {
            SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, texanim[newframe]);
        }
        game.AddDrawTask(*this, DRAW_3D_NOSHADER);
    }
}

bool SlashTest::IsSlashing() const {
    return animtick / game.settings.slashFrameDuration < texanim.size();
}

Vector3 SlashTest::GetPosition() const {
    return Vector3Transform(Vector3Zero(), model.transform);
}

void SlashTest::Draw() {
    DrawModel(model, Vector3Zero(), 1.0f, WHITE);
}

void SlashTest::Launch(const Vector3& from, float direction, float tilt) {
    animtick = -1;
    SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, texanim[0]);

    model.transform =
        MatrixTranslate(1.2f, 0.0f, 0.0f) *
        MatrixRotateX(tilt * DEG2RAD) *
        MatrixRotateY(direction * DEG2RAD) *
        MatrixTranslate(from.x, from.y, from.z);
}

Player::Player(const VoxelMap& v) :
    UpdateTask(PRIO_NORMAL),
    MovingEntity(v, capsule),
    direction(0.0f),
    capsule{}
{
    game.camera.position = { 0.0f, 2.0f, 4.0f };    // Camera position
    game.camera.target = { 0.0f, 1.0f, 0.0f };      // Camera looking at point
    game.camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    game.camera.fovy = 60.0f;                       // Camera field-of-view Y
    game.camera.projection = CAMERA_PERSPECTIVE;    // Camera projection type

    capsule.type = Shape::CAPSULE;
    capsule.capsule = { game.camera.target, { 0.0f, 1.0f, 0.0f }, 0.5f };
    capsule.wantedMask = CollisionMask::SOLID;
    shapeloc = game.collisions.AddShape(capsule);
}

Player::~Player()
{
    game.collisions.RemoveShape(shapeloc);
}

void Player::Do() {
    const float gravity = 0.005f;

    Vector3 prevpos = capsule.position;
    game.camera.target = prevpos;

    Vector2 movement = game.input.movement * (game.settings.playerSpeed * game.deltaTime);
    CameraMoveForward(&game.camera, movement.y, true);
    CameraMoveRight(&game.camera, movement.x, true);
    Vector2 view = game.input.view * (game.deltaTime * 180.0f);
    CameraPitch(&game.camera, -view.y * DEG2RAD, true, true, false);
    CameraYaw(&game.camera, -view.x * DEG2RAD, true);

    Vector3 newpos = game.camera.target;

    if (prevpos != newpos) {
        Vector3 dirvec = newpos - prevpos;
        capsule.position = newpos;
        direction = atan2f(-dirvec.z, dirvec.x) * RAD2DEG;
    }

    ApplyGravity();
    if (grounded && game.input.jump.IsPressed()) {
        velocity.y += 9.0f;
    }
    ApplyVelocity();
    //AdjustToCollisions();

    game.camera.target += capsule.position - newpos;
    game.camera.position += capsule.position - newpos;

    if (game.input.attack.IsPressed()) {
        Vector3 slashpos = capsule.position;
        slashpos.y += 0.7f;
        slash.Launch(slashpos, direction, (float)GetRandomValue(-45, 45));
    }

    game.AddDrawTask(*this, DRAW_3D_SHADER);
    game.AddShadowCaster(*this);
}

void Player::PostCollision()
{
    Vector3 displacement = capsule.position - game.camera.target;

    game.camera.target += displacement;
    game.camera.position += displacement;
}

void Player::Draw() {
    Vector3 tip = capsule.capsule.GetTip();
    DrawCapsuleWires(capsule.position, tip, capsule.capsule.radius, 16, 16, DARKBLUE);

    DrawLine3D(tip, tip + Vector3Transform(Vector3{ 1.0f, 0.0f, 0.0f }, MatrixRotateY(direction * DEG2RAD)), RED);
}