#pragma once
#include "PrimitiveComponent.h"
#include "Define.h"
class UBillboardComponent;

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();
    virtual ~ULightComponentBase() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void InitializeLight();
    
    void SetDiffuseColor(FVector4 NewColor);
    void SetSpecularColor(FVector4 NewColor);
    void SetAttenuation(float Attenuation);
    void SetRange(float r);
    void SetFalloff(float fallOff);

    FVector4 GetDiffuseColor();
    FVector4 GetSpecularColor();
    float GetAttenuation();
    float GetFalloff();
    FLight GetLightInfo() const { return Light; };
protected:

    FBoundingBox AABB;
   
    FLight Light;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    
    float GetRange() const {return Light.Range;}
    
    FVector4 GetColor() const { return FVector4(Light.AmbientColor.X, Light.AmbientColor.Y, Light.AmbientColor.Z, 1); }

};