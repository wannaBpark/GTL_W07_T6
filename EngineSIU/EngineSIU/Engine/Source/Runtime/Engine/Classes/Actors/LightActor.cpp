#include "LightActor.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/BillboardComponent.h"
ALight::ALight()
{
    PointLightComponent = AddComponent<UPointLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");

    PointLightComponent->AttachToComponent(RootComponent);

}

ALight::~ALight()
{
}
