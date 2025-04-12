#include "DirectionalLightActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/BillboardComponent.h"
ADirectionalLight::ADirectionalLight()
{
    DirectionalLightComponent = AddComponent<UDirectionalLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/DirectionalLight_64x.png");

    DirectionalLightComponent->AttachToComponent(RootComponent);
}

ADirectionalLight::~ADirectionalLight()
{
}
