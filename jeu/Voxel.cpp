#include "Voxel.h"
#include "imgui.h"
#include "json.hpp"
#include <fstream>
#include "CollisionMask.h"

using nlohmann::json;

VoxelManager::VoxelManager() : UpdateTask(PRIO_NORMAL), collision{ 0 } {
    Voxel::Position pos{ 0, 0, 0 };
    Voxel& vox = voxels[pos];
    vox.color = RandomColor();
    vox.Init(pos);
}

void VoxelManager::Do() {
    Ray pickray = GetMouseRay({ GetScreenWidth() * 0.5f , GetScreenHeight() * 0.5f }, game.camera);

    collision = GetRayCollisionVoxelMap(pickray, voxels);

    if (!collision.hit) {
        collision = GetRayCollisionQuad(pickray, { -16.0f, 0.0f, 16.0f }, { 16.0f, 0.0f, 16.0f }, { 16.0f, 0.0f, -16.0f }, { -16.0f, 0.0f, -16.0f });
    }

    if (collision.hit) {
        if (game.input.placeblock.IsPressed()) {
            Vector3 pos = collision.point + collision.normal * 0.1f;
            Voxel::Position vpos = Voxel::Position::FromVector3(pos);
            Voxel& vox = voxels[vpos];
            vox.Init(vpos);
            vox.color = RandomColor();
        }
        if (game.input.removeblock.IsPressed()) {
            Vector3 pos = collision.point - collision.normal * 0.1f;
            voxels.erase(Voxel::Position::FromVector3(pos));
        }
    }

    game.AddDrawTask(*this, DRAW_3D_SHADER);
    game.AddShadowCaster(*this);

    // data saving
    ImGui::Begin("Level editor");
    std::string filename = "level.json";
    ImGui::Text("Current level filename : %s", filename.c_str());
    if (ImGui::Button("Load")) {
        std::ifstream in(filename);
        voxels = nlohmann::json::parse(in);
        for (auto& voxel : voxels) {
            voxel.second.Init(voxel.first);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        std::ofstream out(filename, std::ios::out);
        out << json(voxels);
    }
    ImGui::End();
}

void VoxelManager::Draw() {
    DrawSphereWires(collision.point, 0.1f, 8, 8, BLACK);

    for (const auto& voxel : voxels) {
        DrawCube(voxel.first.ToVector3() + Vector3{ 0.5f, 0.5f, 0.5f }, 1.0f, 1.0f, 1.0f, voxel.second.color);
    }
}

RayCollision GetRayCollisionVoxelMap(const Ray& ray, const VoxelMap& voxels)
{
    RayCollision collision{ false, FLT_MAX, Vector3Zero(), Vector3Zero() };

    for (const auto& voxel : voxels) {
        BoundingBox box{ voxel.first.ToVector3(), voxel.first.ToVector3() + Vector3{1.0f,1.0f,1.0f} };
        RayCollision voxelcollision = GetRayCollisionBox(ray, box);
        if (voxelcollision.hit && voxelcollision.distance < collision.distance) {
            collision = voxelcollision;
        }
    }

    return collision;
}

void Voxel::Init(const Position& p)
{
    if (!game.collisions.IsLocationValid(shapeloc)) {
        shape.type = Shape::BOX;
        shape.box = p.ToBox();
        shapeloc = game.collisions.AddShape(shape);
    }
}

Voxel::Voxel() : shapeloc(game.collisions.InvalidLocation())
{
    shape.mask = CollisionMask::SOLID;
}

Voxel::~Voxel()
{
    game.collisions.RemoveShape(shapeloc);
}
