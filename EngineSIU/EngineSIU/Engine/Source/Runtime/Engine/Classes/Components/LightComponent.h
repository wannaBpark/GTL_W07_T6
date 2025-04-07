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
    
    void SetAmbientColor(FVector4 NewColor);
    void SetDiffuseColor(FVector4 NewColor);
    void SetSpecularColor(FVector4 NewColor);
    void SetAttenuation(FVector Attenuation);
    void SetRange(float r);
    void SetFalloff(float fallOff);

    FVector4 GetAmbientColor();
    FVector4 GetDiffuseColor();
    FVector4 GetSpecularColor();
    FVector GetAttenuation();
    float GetFalloff();
    FLight GetLightInfo() const { return Light; };
protected:

    FBoundingBox AABB;
    
    UBillboardComponent* texture2D;

    FLight Light;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    
    float GetRange() const {return Light.Range;}
    
    FVector4 GetColor() const { return FVector4(Light.AmbientColor.X, Light.AmbientColor.Y, Light.AmbientColor.Z, 1); }

    UBillboardComponent* GetTexture2D() const {return texture2D;}
};