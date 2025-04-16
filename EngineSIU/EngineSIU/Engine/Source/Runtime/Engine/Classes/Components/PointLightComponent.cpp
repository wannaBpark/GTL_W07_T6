#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
{
    PointLightInfo.Position = GetWorldLocation();
    PointLightInfo.Radius = 5.0f;

    PointLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    PointLightInfo.Intensity = 1000;
    PointLightInfo.Type = ELightType::POINT_LIGHT;
    PointLightInfo.Attenuation = 20.0f;
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

FLinearColor UPointLightComponent::GetLightColor() const
{
    return PointLightInfo.LightColor;
}

void UPointLightComponent::SetLightColor(const FLinearColor& InColor)
{
    PointLightInfo.LightColor = InColor;
}


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
