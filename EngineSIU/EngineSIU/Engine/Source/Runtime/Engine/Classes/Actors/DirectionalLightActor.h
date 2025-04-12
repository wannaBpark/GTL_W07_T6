#pragma once
#include "LightActor.h"
class ADirectionalLight :
    public ALight
{
    DECLARE_CLASS(ADirectionalLight, ALight)
public:
    ADirectionalLight();
    virtual ~ADirectionalLight();

protected:
    UPROPERTY
    (UDirectionalLightComponent*, DirectionalLightComponent, = nullptr);

    UPROPERTY
    (UBillboardComponent*, BillboardComponent, = nullptr);
};

