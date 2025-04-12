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

    /*
    FVector4 GetDiffuseColor() const;
    void SetDiffuseColor(const FVector4& InColor);

    FVector4 GetSpecularColor() const;
    void SetSpecularColor(const FVector4& InColor);
    */

private:
    FDirectionalLightInfo DirectionalLightInfo;
};

