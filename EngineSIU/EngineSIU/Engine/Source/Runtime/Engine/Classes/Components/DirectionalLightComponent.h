#pragma once
#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponentBase
{

    DECLARE_CLASS(UDirectionalLightComponent, ULightComponentBase)
public:
    UDirectionalLightComponent();
    virtual ~UDirectionalLightComponent() override;
    FVector GetDirection();
    void SetDirection(const FVector& dir);

    const FDirectionalLightInfo& GetDirectionalLightInfo() const;
    void SetDirectionalLightInfo(const FDirectionalLightInfo& InDirectionalLightInfo);

    float GetIntensity() const;
    void SetIntensity(float InIntensity);

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);

private:
    FDirectionalLightInfo DirectionalLightInfo;
};

