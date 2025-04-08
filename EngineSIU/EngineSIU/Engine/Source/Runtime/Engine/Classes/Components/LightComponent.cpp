#include "LightComponent.h"
#include "UBillboardComponent.h"


ULightComponentBase::ULightComponentBase()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponentBase::~ULightComponentBase()
{
  
}

void ULightComponentBase::SetDiffuseColor(FVector4 NewColor)
{
    Light.DiffuseColor = FVector(NewColor.X, NewColor.Y, NewColor.Z);
}

void ULightComponentBase::SetSpecularColor(FVector4 NewColor)
{
   Light.SpecularColor = FVector(NewColor.X, NewColor.Y, NewColor.Z);
}

void ULightComponentBase::SetAttenuation(float Attenuation)
{
    Light.Attenuation = Attenuation;
}

void ULightComponentBase::SetRange(float r)
{
    Light.Range = r;
}

void ULightComponentBase::SetFalloff(float fallOff)
{
    Light.Falloff = fallOff;
}

FVector4 ULightComponentBase::GetDiffuseColor()
{
    return FVector4(Light.DiffuseColor, 1);
}

FVector4 ULightComponentBase::GetSpecularColor()
{
    return FVector4(Light.SpecularColor, 1);
}

float ULightComponentBase::GetAttenuation()
{
    return Light.Attenuation;
}

float ULightComponentBase::GetFalloff()
{
    return Light.Falloff;
}

void ULightComponentBase::InitializeLight()
{  
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    
    Light = FLight();
    Light.Enabled = 1;
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res =AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

