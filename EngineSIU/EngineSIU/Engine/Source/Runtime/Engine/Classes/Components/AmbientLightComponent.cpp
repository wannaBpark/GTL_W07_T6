#include "AmbientLightComponent.h"

UAmbientLightComponent::UAmbientLightComponent()
{
    AmbientLightInfo.AmbientColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //AmbientLightInfo.AmbientColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
}

UAmbientLightComponent::~UAmbientLightComponent()
{
}

const FAmbientLightInfo& UAmbientLightComponent::GetAmbientLightInfo() const
{
    return AmbientLightInfo;
}

void UAmbientLightComponent::SetAmbientLightInfo(const FAmbientLightInfo& InAmbient)
{
    AmbientLightInfo = InAmbient;
}

FLinearColor UAmbientLightComponent::GetLightColor() const
{
    return AmbientLightInfo.AmbientColor;
}

void UAmbientLightComponent::SetLightColor(const FLinearColor& InColor)
{
    AmbientLightInfo.AmbientColor = InColor;
}
