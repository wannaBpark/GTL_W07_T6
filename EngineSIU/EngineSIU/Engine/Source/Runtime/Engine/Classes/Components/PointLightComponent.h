#pragma once
#include "LightComponent.h"

class UPointLightComponent :public ULightComponentBase
{

    DECLARE_CLASS(UPointLightComponent, ULightComponentBase)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    const FPointLightInfo& GetPointLightInfo() const;
    void SetPointLightInfo(const FPointLightInfo& InPointLightInfo);

    float GetRadius() const;
    void SetRadius(float InRadius);

    /*
    FVector4 GetDiffuseColor() const;
    void SetDiffuseColor(const FVector4& InColor);

    FVector4 GetSpecularColor() const;
    void SetSpecularColor(const FVector4& InColor);
    */

    float GetIntensity() const;
    void SetIntensity(float InIntensity);

    int GetType() const;
    void SetType(int InType);

private:
    FPointLightInfo PointLightInfo;
};


