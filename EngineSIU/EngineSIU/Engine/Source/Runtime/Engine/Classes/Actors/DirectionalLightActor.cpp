#include "DirectionalLightActor.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/BillboardComponent.h"
ADirectionalLight::ADirectionalLight()
{
    DirectionalLightComponent = AddComponent<UDirectionalLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/DirectionalLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    DirectionalLightComponent->AttachToComponent(RootComponent);
}

ADirectionalLight::~ADirectionalLight()
{
}

void ADirectionalLight::SetIntensity(float Intensity)
{
    if (DirectionalLightComponent)
    {
        DirectionalLightComponent->SetIntensity(Intensity);
    }
}
