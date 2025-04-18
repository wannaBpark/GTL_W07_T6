#include "SpotLightActor.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/BillboardComponent.h"
ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/SpotLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    SpotLightComponent->AttachToComponent(RootComponent);
}

ASpotLight::~ASpotLight()
{
}
