#include "DirectionalLightComponent.h"
#include "Components/SceneComponent.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    Light.Type = ELightType::DIRECTIONAL_LIGHT;
}

UDirectionalLightComponent::~UDirectionalLightComponent()
{
}

FVector UDirectionalLightComponent::GetDirection()  
{  
  return -GetUpVector();  
}

void UDirectionalLightComponent::SetDirection(const FVector& dir)
{
    Light.Direction = dir;
}
