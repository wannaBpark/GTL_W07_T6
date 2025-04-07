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
    void SetColor(FVector4 NewColor);
  
    void SetRadius(float r);

    FLight GetLightInfo() const { return Light; };
private:

    FBoundingBox AABB;
    
    UBillboardComponent* texture2D;

    FLight Light;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    
    float GetRadius() const {return Light.Range;}
    
    FVector4 GetColor() const { return FVector4(Light.AmbientColor.X, Light.AmbientColor.Y, Light.AmbientColor.Z, 1); }

    UBillboardComponent* GetTexture2D() const {return texture2D;}
};