#include "PointLightActor.h"
#include "Components/PointLightComponent.h"
#include "Components/BillboardComponent.h"

APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");

    PointLightComponent->AttachToComponent(RootComponent);
}

APointLight::~APointLight()
{
}
