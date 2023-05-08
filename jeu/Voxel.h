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

        inline Box ToBox() const {
            return { {(float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f}, { 0.5f, 0.5f, 0.5f} };
        }

        static Position FromVector3(const Vector3& vec) {
            return { (int)std::floor(vec.x), (int)std::floor(vec.y), (int)std::floor(vec.z) };
        }
    };

    Color color{};
    Shape shape{};
    ShapeLocation shapeloc{};

    void Init(const Position& p);
    Voxel();
    ~Voxel();
};

typedef std::map<Voxel::Position, Voxel> VoxelMap;

class VoxelManager : public UpdateTask, public IDrawTask {
private:
    VoxelMap voxels;
    RayCollision collision;

public:
    VoxelManager();

    inline const VoxelMap& GetVoxels() const {
        return voxels;
    }

    void Do();
    void Draw();
};

RayCollision GetRayCollisionVoxelMap(const Ray& ray, const VoxelMap& voxels);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Voxel::Position, x, y, z);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Voxel, color);