#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
{
    Light.Type = ELightType::POINT_LIGHT;

    PointLightInfo.Position = GetWorldLocation();
    PointLightInfo.Radius = 30;

    PointLightInfo.DiffuseColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    PointLightInfo.SpecularColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);

    PointLightInfo.Intensity = 10;
    PointLightInfo.Type = ELightType::POINT_LIGHT;
}

UPointLightComponent::~UPointLightComponent()
{
}

const FPointLightInfo& UPointLightComponent::GetPointLightInfo() const
{
    return PointLightInfo;
}

void UPointLightComponent::SetPointLightInfo(const FPointLightInfo& InPointLightInfo)
{
    PointLightInfo = InPointLightInfo;
}


float UPointLightComponent::GetRadius() const
{
    return PointLightInfo.Radius;
}

void UPointLightComponent::SetRadius(float InRadius)
{
    PointLightInfo.Radius = InRadius;
}
/*
FVector4 UPointLightComponent::GetDiffuseColor() const
{
    return PointLightInfo.DiffuseColor;
}

void UPointLightComponent::SetDiffuseColor(const FVector4& InColor)
{
    PointLightInfo.DiffuseColor = InColor;
}

FVector4 UPointLightComponent::GetSpecularColor() const
{
    return PointLightInfo.SpecularColor;
}

void UPointLightComponent::SetSpecularColor(const FVector4& InColor)
{
    PointLightInfo.SpecularColor = InColor;
}
*/

float UPointLightComponent::GetIntensity() const
{
    return PointLightInfo.Intensity;
}

void UPointLightComponent::SetIntensity(float InIntensity)
{
    PointLightInfo.Intensity = InIntensity;
}

int UPointLightComponent::GetType() const
{
    return PointLightInfo.Type;
}

void UPointLightComponent::SetType(int InType)
{
    PointLightInfo.Type = InType;
}
