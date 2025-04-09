#pragma once
#include "Serialization/Archive.h"


#include "Matrix.h"
// 쿼터니언
struct FQuat
{
    float W, X, Y, Z;

    // 기본 생성자
    FQuat()
        : W(1.0f)
        , X(0.0f)
        , Y(0.0f)
        , Z(0.0f)
    {
    }

    // FQuat 생성자 추가: 회전 축과 각도를 받아서 FQuat 생성
    FQuat(const FVector& Axis, float Angle)
    {
        float HalfAngle = Angle * 0.5f;
        float SinHalfAngle = sinf(HalfAngle);
        float CosHalfAngle = cosf(HalfAngle);

        X = Axis.X * SinHalfAngle;
        Y = Axis.Y * SinHalfAngle;
        Z = Axis.Z * SinHalfAngle;
        W = CosHalfAngle;
    }

    // W, X, Y, Z 값으로 초기화
    FQuat(float InW, float InX, float InY, float InZ)
        : W(InW)
        , X(InX)
        , Y(InY)
        , Z(InZ)
    {
    }

    // 쿼터니언의 곱셈 연산 (회전 결합)
    FQuat operator*(const FQuat& Other) const
    {
        return FQuat(
            W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z,
            W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y,
            W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X,
            W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W
        );
    }

    // (쿼터니언) 벡터 회전
    FVector RotateVector(const FVector& Vec) const
    {
        // 벡터를 쿼터니언으로 변환
        FQuat vecQuat(0.0f, Vec.X, Vec.Y, Vec.Z);
        // 회전 적용 (q * vec * q^-1)
        FQuat conjugate = FQuat(W, -X, -Y, -Z); // 쿼터니언의 켤레
        FQuat result = *this * vecQuat * conjugate;

        return FVector(result.X, result.Y, result.Z); // 회전된 벡터 반환
    }

    // 단위 쿼터니언 여부 확인
    bool IsNormalized() const
    {
        return fabs(W * W + X * X + Y * Y + Z * Z - 1.0f) < 1e-6f;
    }

    // 쿼터니언 정규화 (단위 쿼터니언으로 만듬)
    FQuat Normalize() const
    {
        float magnitude = sqrtf(W * W + X * X + Y * Y + Z * Z);
        return FQuat(W / magnitude, X / magnitude, Y / magnitude, Z / magnitude);
    }

    // 회전 각도와 축으로부터 쿼터니언 생성 (axis-angle 방식)
    static FQuat FromAxisAngle(const FVector& Axis, float Angle)
    {
        float halfAngle = Angle * 0.5f;
        float sinHalfAngle = sinf(halfAngle);
        return FQuat(cosf(halfAngle), Axis.X * sinHalfAngle, Axis.Y * sinHalfAngle, Axis.Z * sinHalfAngle);
    }

    static FQuat CreateRotation(float roll, float pitch, float yaw)
    {
        // 각도를 라디안으로 변환
        float radRoll = roll * (PI / 180.0f);
        float radPitch = pitch * (PI / 180.0f);
        float radYaw = yaw * (PI / 180.0f);

        // 각 축에 대한 회전 쿼터니언 계산
        FQuat qRoll = FQuat(FVector(1.0f, 0.0f, 0.0f), radRoll); // X축 회전
        FQuat qPitch = FQuat(FVector(0.0f, 1.0f, 0.0f), radPitch); // Y축 회전
        FQuat qYaw = FQuat(FVector(0.0f, 0.0f, 1.0f), radYaw); // Z축 회전

        // 회전 순서대로 쿼터니언 결합 (Y -> X -> Z)
        return qRoll * qPitch * qYaw;
    }

    // 쿼터니언을 회전 행렬로 변환
    FMatrix ToMatrix() const
    {
        FMatrix RotationMatrix;
        RotationMatrix.M[0][0] = 1.0f - 2.0f * (Y * Y + Z * Z);
        RotationMatrix.M[0][1] = 2.0f * (X * Y - W * Z);
        RotationMatrix.M[0][2] = 2.0f * (X * Z + W * Y);
        RotationMatrix.M[0][3] = 0.0f;


        RotationMatrix.M[1][0] = 2.0f * (X * Y + W * Z);
        RotationMatrix.M[1][1] = 1.0f - 2.0f * (X * X + Z * Z);
        RotationMatrix.M[1][2] = 2.0f * (Y * Z - W * X);
        RotationMatrix.M[1][3] = 0.0f;

        RotationMatrix.M[2][0] = 2.0f * (X * Z - W * Y);
        RotationMatrix.M[2][1] = 2.0f * (Y * Z + W * X);
        RotationMatrix.M[2][2] = 1.0f - 2.0f * (X * X + Y * Y);
        RotationMatrix.M[2][3] = 0.0f;

        RotationMatrix.M[3][0] = RotationMatrix.M[3][1] = RotationMatrix.M[3][2] = 0.0f;
        RotationMatrix.M[3][3] = 1.0f;

        return RotationMatrix;
    }
};

inline FArchive& operator<<(FArchive& Ar, FQuat& Q)
{
    return Ar << Q.X << Q.Y << Q.Z << Q.W;
}
