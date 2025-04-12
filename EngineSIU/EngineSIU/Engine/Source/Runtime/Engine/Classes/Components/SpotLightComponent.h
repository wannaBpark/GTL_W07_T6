#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponentBase
{

    DECLARE_CLASS(USpotLightComponent, ULightComponentBase)
public:
    USpotLightComponent();
    virtual ~USpotLightComponent();
    FVector GetDirection();
    void SetDirection(const FVector& dir);

    const FSpotLightInfo& GetSpotLightInfo() const;
    void SetSpotLightInfo(const FSpotLightInfo& InSpotLightInfo);

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

    float GetInnerCos() const;
    void SetInnerCos(float InInnerCos);

    float GetOuterCos() const;
    void SetOuterCos(float InOuterCos);
private:
    FSpotLightInfo SpotLightInfo;
};

