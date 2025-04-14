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

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);



    float GetIntensity() const;
    void SetIntensity(float InIntensity);

    int GetType() const;
    void SetType(int InType);

    float GetInnerRad() const;
    void SetInnerRad(float InInnerCos);

    float GetOuterRad() const;
    void SetOuterRad(float InOuterCos);
private:
    FSpotLightInfo SpotLightInfo;
};

