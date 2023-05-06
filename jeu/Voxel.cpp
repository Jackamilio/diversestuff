#include "Voxel.h"
#include "imgui.h"
#include "json.hpp"
#include <fstream>

using nlohmann::json;

VoxelManager::VoxelManager() : UpdateTask(PRIO_NORMAL), collision{ 0 } {
    voxels[{0, 0, 0}].color = RandomColor();
}

void VoxelManager::Do() {
    collision.hit = false;
    collision.distance = FLT_MAX;

    Ray pickray = GetMouseRay({ GetScreenWidth() * 0.5f , GetScreenHeight() * 0.5f }, game.camera);

    for (const auto& voxel : voxels) {
        BoundingBox box{ voxel.first.ToVector3(), voxel.first.ToVector3() + Vector3{1.0f,1.0f,1.0f} };
        RayCollision voxelcollision = GetRayCollisionBox(pickray, box);
        if (voxelcollision.hit && voxelcollision.distance < collision.distance) {
            collision = voxelcollision;
        }
    }

    if (!collision.hit) {
        collision = GetRayCollisionQuad(pickray, { -16.0f, 0.0f, 16.0f }, { 16.0f, 0.0f, 16.0f }, { 16.0f, 0.0f, -16.0f }, { -16.0f, 0.0f, -16.0f });
    }

    if (collision.hit) {
        if (game.input.placeblock.IsPressed()) {
            Vector3 pos = collision.point + collision.normal * 0.1f;
            voxels[Voxel::Position::FromVector3(pos)].color = RandomColor();
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