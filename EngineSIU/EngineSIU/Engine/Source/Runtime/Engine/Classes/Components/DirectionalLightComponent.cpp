#include "DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    Light.Type = ELightType::DIRECTIONAL_LIGHT;

    DirectionalLightInfo.Direction = -GetUpVector();
    DirectionalLightInfo.Intensity = 10.0f;

    DirectionalLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

UDirectionalLightComponent::~UDirectionalLightComponent()
{
}


FVector UDirectionalLightComponent::GetDirection()  
{
    FRotator rotator = GetWorldRotation();
    FVector WorldDown= rotator.ToQuaternion().RotateVector(-GetUpVector());
    return WorldDown;  
}

void UDirectionalLightComponent::SetDirection(const FVector& dir)
{
    Light.Direction = dir;
}

const FDirectionalLightInfo& UDirectionalLightComponent::GetDirectionalLightInfo() const
{
    return DirectionalLightInfo;
}

void UDirectionalLightComponent::SetDirectionalLightInfo(const FDirectionalLightInfo& InDirectionalLightInfo)
{
    DirectionalLightInfo = InDirectionalLightInfo;
}

float UDirectionalLightComponent::GetIntensity() const
{
    return DirectionalLightInfo.Intensity;
}

void UDirectionalLightComponent::SetIntensity(float InIntensity)
{
    DirectionalLightInfo.Intensity = InIntensity;
}
/*
FVector4 UDirectionalLightComponent::GetDiffuseColor() const
{
    return DirectionalLightInfo.DiffuseColor;
}

void UDirectionalLightComponent::SetDiffuseColor(const FVector4& InColor)
{
    DirectionalLightInfo.DiffuseColor = InColor;
}

FVector4 UDirectionalLightComponent::GetSpecularColor() const
{
    return DirectionalLightInfo.SpecularColor;
}

void UDirectionalLightComponent::SetSpecularColor(const FVector4& InColor)
{
    DirectionalLightInfo.SpecularColor = InColor;
}
*/
