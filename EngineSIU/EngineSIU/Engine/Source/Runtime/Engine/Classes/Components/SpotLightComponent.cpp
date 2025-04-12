#include "SpotLightComponent.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
USpotLightComponent::USpotLightComponent()
{
    Light.Type = ELightType::SPOT_LIGHT;
    Light.InnerCos = 0.9659;
    Light.OuterCos = 0.8660;

    SpotLightInfo.Position = GetWorldLocation();
    SpotLightInfo.Radius = 30.0f;
    SpotLightInfo.Direction = GetForwardVector();
    SpotLightInfo.DiffuseColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    SpotLightInfo.SpecularColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    SpotLightInfo.Intensity = 10.0f;
    SpotLightInfo.Type = ELightType::SPOT_LIGHT;
    SpotLightInfo.InnerRad = 0.9659;
    SpotLightInfo.OuterRad = 0.8660;
}

USpotLightComponent::~USpotLightComponent()
{
}


FVector USpotLightComponent::GetDirection()
{
    FRotator rotator = GetWorldRotation();
    FVector WorldForward = rotator.ToQuaternion().RotateVector(GetForwardVector());
    return WorldForward;
}

void USpotLightComponent::SetDirection(const FVector& dir)
{
    Light.Direction = dir;
}

const FSpotLightInfo& USpotLightComponent::GetSpotLightInfo() const
{
    return SpotLightInfo;
}

void USpotLightComponent::SetSpotLightInfo(const FSpotLightInfo& InSpotLightInfo)
{
    SpotLightInfo = InSpotLightInfo;
}

float USpotLightComponent::GetRadius() const
{
    return SpotLightInfo.Radius;
}

void USpotLightComponent::SetRadius(float InRadius)
{
    SpotLightInfo.Radius = InRadius;
}
/*
FVector4 USpotLightComponent::GetDiffuseColor() const
{
    return SpotLightInfo.DiffuseColor;
}

void USpotLightComponent::SetDiffuseColor(const FVector4& InColor)
{
    SpotLightInfo.DiffuseColor = InColor;
}

FVector4 USpotLightComponent::GetSpecularColor() const
{
    return SpotLightInfo.SpecularColor;
}

void USpotLightComponent::SetSpecularColor(const FVector4& InColor)
{
    SpotLightInfo.SpecularColor = InColor;
}
*/

float USpotLightComponent::GetIntensity() const
{
    return SpotLightInfo.Intensity;
}

void USpotLightComponent::SetIntensity(float InIntensity)
{
    SpotLightInfo.Intensity = InIntensity;
}

int USpotLightComponent::GetType() const
{
    return SpotLightInfo.Type;
}

void USpotLightComponent::SetType(int InType)
{
    SpotLightInfo.Type = InType;
}

float USpotLightComponent::GetInnerRad() const
{
    return SpotLightInfo.InnerRad;
}

void USpotLightComponent::SetInnerRad(float InInnerCos)
{
    SpotLightInfo.InnerRad = InInnerCos;
}

float USpotLightComponent::GetOuterRad() const
{
    return SpotLightInfo.OuterRad;
}

void USpotLightComponent::SetOuterRad(float InOuterCos)
{
    SpotLightInfo.OuterRad = InOuterCos;
}
