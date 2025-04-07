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
    delete texture2D;
}
void ULightComponentBase::SetColor(FVector4 NewColor)
{
    Light.AmbientColor = FVector(NewColor.X, NewColor.Y, NewColor.Z);
}

void ULightComponentBase::SetRadius(float r)
{
    Light.Range = r;
}

void ULightComponentBase::InitializeLight()
{
    texture2D = new UBillboardComponent();
    texture2D->SetTexture(L"Assets/Texture/spotLight.png");
    texture2D->InitializeComponent();
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };

}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    texture2D->TickComponent(DeltaTime);
    texture2D->SetRelativeLocation(GetWorldLocation());

}

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res =AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

