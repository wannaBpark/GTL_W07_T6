#pragma once
#include "Vector.h"
// 4D Vector
struct FVector4 {
    float X, Y, Z, W;
    FVector4(float _x = 0, float _y = 0, float _z = 0, float _a = 0) : X(_x), Y(_y), Z(_z), W(_a) {}

    FVector4(FVector _vector, float _a = 0) : X(_vector.X), Y(_vector.Y), Z(_vector.Z), W(_a) {}

    FVector4 operator-(const FVector4& other) const {
        return FVector4(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
    }
    FVector4 operator+(const FVector4& other) const {
        return FVector4(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
    }
    FVector4 operator/(float scalar) const
    {
        return FVector4{ X / scalar, Y / scalar, Z / scalar, W / scalar };
    }
};
