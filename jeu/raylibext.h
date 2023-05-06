#pragma once
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include "json.hpp"

inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
    return Vector3Add(lhs, rhs);
}

inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs) {
    return Vector3Subtract(lhs, rhs);
}

inline Vector3 operator*(const Vector3& lhs, const float scalar) {
    return Vector3Scale(lhs, scalar);
}

inline Vector3 operator/(const Vector3& lhs, const float scalar) {
    return Vector3Scale(lhs, 1.0f/scalar);
}

inline Vector3& operator+=(Vector3& lhs, const Vector3& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

inline Vector3& operator-=(Vector3& lhs, const Vector3& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

inline Vector3& operator+=(Vector3& lhs, float rhs) {
    lhs.x += rhs;
    lhs.y += rhs;
    lhs.z += rhs;
    return lhs;
}

inline Vector3& operator-=(Vector3& lhs, float rhs) {
    lhs.x -= rhs;
    lhs.y -= rhs;
    lhs.z -= rhs;
    return lhs;
}

inline Vector3 operator-(const Vector3& rhs) {
    return {-rhs.x, -rhs.y, -rhs.z};
}

inline bool operator==(const Vector3& lhs, const Vector3& rhs) {
    return Vector3Equals(lhs, rhs);
}

inline bool operator!=(const Vector3& lhs, const Vector3& rhs) {
    return !Vector3Equals(lhs, rhs);
}

inline Matrix operator*(const Matrix& lhs, const Matrix& rhs) {
    return MatrixMultiply(lhs, rhs);
}

// makes sure each min component is < to each max component
BoundingBox FixBoundingBox(const BoundingBox& bbtofix);

// Load custom render texture, create a writable depth texture buffer
RenderTexture2D LoadRenderTextureDepthTex(int width, int height);

// Unload render texture from GPU memory (VRAM)
void UnloadRenderTextureDepthTex(RenderTexture2D target);

typedef struct Segment {
    Vector3 pointA;
    Vector3 pointB;

    inline BoundingBox GetBoundingBox() const {
        return FixBoundingBox({ pointA, pointB });
    }
    inline Vector3 Lerp(float t) const {
        return pointA + (pointB - pointA) * t;
    }
} Segment;

typedef struct Sphere {
    Vector3 center;
    float radius;
    inline BoundingBox GetBoundingBox() const {
        return {Vector3SubtractValue(center, radius), Vector3AddValue(center, radius)};
    }
}Sphere;

typedef struct Capsule {
    Vector3 base; // base position in world
    Vector3 offset; // second extremity relative to base
    float radius;
    inline BoundingBox GetBoundingBox() const {
        BoundingBox segmentbb = FixBoundingBox({ base, GetTip() });
        segmentbb.min -= radius;
        segmentbb.max += radius;
    }
    inline Vector3 GetCenter() const {
        return base + offset * 0.5f;
    }
    inline Vector3 GetTip() const {
        return base + offset;
    }
    inline float radiussqr() const {
        return radius * radius;
    }
    inline Segment GetSegment() const {
        return {base, base + offset};
    }
} Capsule;

inline Color RandomColor() {
    return { (unsigned char)GetRandomValue(0,255), (unsigned char)GetRandomValue(0,255) , (unsigned char)GetRandomValue(0,255), 255 };
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Vector2, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Vector3, x, y, z);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Vector4, x, y, z, w);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Color, r, g, b, a);