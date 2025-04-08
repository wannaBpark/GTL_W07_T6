#pragma once

// 회전 정보를 Degree 단위로 저장하는 구조체
struct FRotator
{
    float Pitch;
    float Yaw;
    float Roll;

    FRotator()
        : Pitch(0.0f)
        , Yaw(0.0f)
        , Roll(0.0f)
    {
    }

    FRotator(float InPitch, float InYaw, float InRoll)
        : Pitch(InPitch)
        , Yaw(InYaw)
        , Roll(InRoll)
    {
    }

    FRotator(const FRotator& Other)
        : Pitch(Other.Pitch)
        , Yaw(Other.Yaw)
        , Roll(Other.Roll)
    {
    }

    FRotator(const FVector& InVector)
        : Pitch(FMath::RadiansToDegrees(InVector.Y))
        , Yaw(FMath::RadiansToDegrees(InVector.Z))
        , Roll(FMath::RadiansToDegrees(InVector.X))
    {
    }

    FRotator(const FQuat& InQuat);

    FRotator operator+(const FRotator& Other) const;
    FRotator& operator+=(const FRotator& Other);

    FRotator operator-(const FRotator& Other) const;
    FRotator& operator-=(const FRotator& Other);

    FRotator operator*(float Scalar) const;
    FRotator& operator*=(float Scalar);

    FRotator operator/(const FRotator& Other) const;
    FRotator operator/(float Scalar) const;
    FRotator& operator/=(float Scalar);

    FRotator operator-() const;

    bool operator==(const FRotator& Other) const;
    bool operator!=(const FRotator& Other) const;

    bool IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const;
    bool IsZero() const;

    bool Equals(const FRotator& Other, float Tolerance = KINDA_SMALL_NUMBER) const;

    FRotator Add(float DeltaPitch, float DeltaYaw, float DeltaRoll) const;

    FRotator FromQuaternion(const FQuat& InQuat) const;
    FQuat ToQuaternion() const;
    FVector ToVector() const;
    FMatrix ToMatrix() const;

    float Clamp(float Angle) const;
    FRotator GetNormalized() const;
    void Normalize();
};

FORCEINLINE FRotator::FRotator(const FQuat& InQuat)
{
    const float SingularityTest = InQuat.Z * InQuat.X - InQuat.W * InQuat.Y;
    const float YawY = 2.f * (InQuat.W * InQuat.Z + InQuat.X * InQuat.Y);
    const float YawX = (1.f - 2.f * (FMath::Square(InQuat.Y) + FMath::Square(InQuat.Z)));

    // reference 
    // http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

    // this value was found from experience, the above websites recommend different values
    // but that isn't the case for us, so I went through different testing, and finally found the case 
    // where both of world lives happily. 
    const float SINGULARITY_THRESHOLD = 0.4999995f;
    const float RAD_TO_DEG = (180.f / PI);

    if (SingularityTest < -SINGULARITY_THRESHOLD)
    {
        Pitch = -90.f;
        Yaw = (FMath::Atan2(YawY, YawX) * RAD_TO_DEG);
        Roll = (-Yaw - (2.f * atan2(InQuat.X, InQuat.W) * RAD_TO_DEG));
    }
    else if (SingularityTest > SINGULARITY_THRESHOLD)
    {
        Pitch = 90.f;
        Yaw = (FMath::Atan2(YawY, YawX) * RAD_TO_DEG);
        Roll = (Yaw - (2.f * FMath::Atan2(InQuat.X, InQuat.W) * RAD_TO_DEG));
    }
    else
    {
        Pitch = (asin(2.f * SingularityTest) * RAD_TO_DEG);
        Yaw = (FMath::Atan2(YawY, YawX) * RAD_TO_DEG);
        Roll = (FMath::Atan2(-2.f * (InQuat.W * InQuat.X + InQuat.Y * InQuat.Z),
                             (1.f - 2.f * (FMath::Square(InQuat.X) + FMath::Square(InQuat.Y)))) * RAD_TO_DEG);
    }
}

inline FRotator FRotator::operator+(const FRotator& Other) const
{
    return FRotator(Pitch + Other.Pitch, Yaw + Other.Yaw, Roll + Other.Roll);
}

inline FRotator& FRotator::operator+=(const FRotator& Other)
{
    Pitch += Other.Pitch; Yaw += Other.Yaw; Roll += Other.Roll;
    return *this;
}

inline FRotator FRotator::operator-(const FRotator& Other) const
{
    return { Pitch - Other.Pitch, Yaw - Other.Yaw, Roll - Other.Roll };
}

inline FRotator& FRotator::operator-=(const FRotator& Other)
{
    Pitch -= Other.Pitch; Yaw -= Other.Yaw; Roll -= Other.Roll;
    return *this;
}

inline FRotator FRotator::operator*(float Scalar) const
{
    return { Pitch * Scalar, Yaw * Scalar, Roll * Scalar };
}

inline FRotator& FRotator::operator*=(float Scalar)
{
    Pitch *= Scalar; Yaw *= Scalar; Roll *= Scalar;
    return *this;
}

inline FRotator FRotator::operator/(const FRotator& Other) const
{
    return { Pitch / Other.Pitch, Yaw / Other.Yaw, Roll / Other.Roll };
}

inline FRotator FRotator::operator/(float Scalar) const
{
    return { Pitch / Scalar, Yaw / Scalar, Roll / Scalar };
}

inline FRotator& FRotator::operator/=(float Scalar)
{
    Pitch /= Scalar; Yaw /= Scalar; Roll /= Scalar;
    return *this;
}

inline FRotator FRotator::operator-() const
{
    return { -Pitch, -Yaw, -Roll };
}

inline bool FRotator::operator==(const FRotator& Other) const
{
    return Pitch == Other.Pitch && Yaw == Other.Yaw && Roll == Other.Roll;
}

inline bool FRotator::operator!=(const FRotator& Other) const
{
    return !(*this == Other);
}

inline FRotator FRotator::FromQuaternion(const FQuat& InQuat) const
{
    return FRotator(InQuat);
}


inline FQuat FRotator::ToQuaternion() const
{
    float DegToRad = PI / 180.0f;
    float Div =  DegToRad / 2.0f;
    float SP, SY, SR;
    float CP, CY, CR;

    FMath::SinCos(&SP, &CP, Pitch * Div);
    FMath::SinCos(&SY, &CY, Yaw * Div);
    FMath::SinCos(&SR, &CR, Roll * Div);
	
    FQuat RotationQuat;
    RotationQuat.X = CR * SP * SY - SR * CP * CY;
    RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
    RotationQuat.Z = CR * CP * SY - SR * SP * CY;
    RotationQuat.W = CR * CP * CY + SR * SP * SY;

    return RotationQuat;
}

inline FVector FRotator::ToVector() const
{
    return FVector(FMath::DegreesToRadians(Roll), FMath::DegreesToRadians(Pitch), FMath::DegreesToRadians(Yaw));
}

