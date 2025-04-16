#include "AmbientLightComponent.h"

UAmbientLightComponent::UAmbientLightComponent()
{
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
