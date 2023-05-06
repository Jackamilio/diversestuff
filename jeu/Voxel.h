#pragma once

#include "Game.h"
#include "raylibext.h"
#include <cmath>
#include <map>
#include "json.hpp"

struct Voxel {
    struct Position {
        int x;
        int y;
        int z;

        const bool operator< (const Position& rhs) const {
            if (x == rhs.x) {
                if (y == rhs.y) {
                    return z < rhs.z;
                }
                return y < rhs.y;
            }
            return x < rhs.x;
        }

        inline Vector3 ToVector3() const {
            return { (float)x, (float)y, (float)z };
        }

        inline BoundingBox ToBoundingBox() const {
            return { {(float)x, (float)y, (float)z},  {(float)(x + 1), (float)(y + 1), (float)(z + 1)} };
        }

        static Position FromVector3(const Vector3& vec) {
            return { (int)std::floor(vec.x), (int)std::floor(vec.y), (int)std::floor(vec.z) };
        }
    };

    Color color;
};

typedef std::map<Voxel::Position, Voxel> VoxelMap;

class VoxelManager : public UpdateTask, public IDrawTask {
public:
    VoxelMap voxels;
    RayCollision collision;

    VoxelManager();

    void Do();
    void Draw();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Voxel::Position, x, y, z);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Voxel, color);