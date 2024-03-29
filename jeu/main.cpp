#include "Game.h"
#include "Voxel.h"
#include "MovingEntity.h"
#include "Player.h"
#include "Enemy.h"
#include "PointLight.h"
#include "CollisionMask.h"

#include <iostream>

class GamePlane : public DrawTask {
public:
    ShapeLocation shapeloc;

    GamePlane() : DrawTask(DRAW_3D_SHADER)
    {
        shapeloc = game.collisions.AddPlane({ 0.0f, 1.0f, 0.0f, 0.0f }, CollisionMask::SOLID, 0);
    }
    ~GamePlane() {
        game.collisions.RemoveShape(shapeloc);
    }

    void Draw() {
        DrawPlane({ 0.0f, 0.0f, 0.0f }, { 32.0f, 32.0f }, LIGHTGRAY);
    }
};

int main(void)
{
    game.Init();

    GamePlane gp;
    VoxelManager vm;
    Player p(vm.GetVoxels());
    Enemy e(vm.GetVoxels(), Vector3{-10.0f, 2.0f, -10.0f}, p.shapeloc->position, p.slash);
    PointLight pl;

    game.Loop();

    game.Quit();

    return 0;
}
