#include "AmbientLightComponent.h"

UAmbientLightComponent::UAmbientLightComponent()
{
    Light.Type = ELightType::AMBIENT_LIGHT;
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
