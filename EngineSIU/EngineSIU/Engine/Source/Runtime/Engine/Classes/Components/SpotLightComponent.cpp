#include "SpotLightComponent.h"
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
    SpotLightInfo.InnerCos = 0.9659;
    SpotLightInfo.OuterCos = 0.8660;
}

USpotLightComponent::~USpotLightComponent()
{
}


FVector USpotLightComponent::GetDirection()
{
    return Light.Direction;
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

float USpotLightComponent::GetInnerCos() const
{
    return SpotLightInfo.InnerCos;
}

void USpotLightComponent::SetInnerCos(float InInnerCos)
{
    SpotLightInfo.InnerCos = InInnerCos;
}

float USpotLightComponent::GetOuterCos() const
{
    return SpotLightInfo.OuterCos;
}

void USpotLightComponent::SetOuterCos(float InOuterCos)
{
    SpotLightInfo.OuterCos = InOuterCos;
}
