#include "Game.h"
#include "Voxel.h"
#include "MovingEntity.h"
#include "Player.h"
#include "Enemy.h"
#include "PointLight.h"

#include <iostream>

class GamePlane : public DrawTask {
public:
    GamePlane() : DrawTask(DRAW_3D_SHADER) {}

    void Draw() {
        DrawPlane({ 0.0f, 0.0f, 0.0f }, { 32.0f, 32.0f }, LIGHTGRAY);
    }
};

int main(void)
{
    game.Init();

    GamePlane gp;
    VoxelManager vm;
    Player p(vm.voxels);
    Enemy e(vm.voxels, Vector3{-10.0f, 2.0f, -10.0f}, p.position, p.slash);
    PointLight pl;

    game.Loop();

    game.Quit();

    return 0;
}