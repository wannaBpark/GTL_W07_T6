#pragma once
#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponentBase
{

    DECLARE_CLASS(UDirectionalLightComponent, ULightComponentBase)
public:
    UDirectionalLightComponent();
    virtual ~UDirectionalLightComponent() override;
};

