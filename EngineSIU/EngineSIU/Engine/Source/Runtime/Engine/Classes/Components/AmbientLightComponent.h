#pragma once
#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponentBase
{

    DECLARE_CLASS(UAmbientLightComponent, ULightComponentBase)
public:
    UAmbientLightComponent();
    virtual ~UAmbientLightComponent() override;

};
