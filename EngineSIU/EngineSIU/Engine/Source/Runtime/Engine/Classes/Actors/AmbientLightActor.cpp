#include "AmbientLightActor.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Components/BillboardComponent.h"

AAmbientLight::AAmbientLight()
{
    AmbientLightComponent = AddComponent<UAmbientLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/AmbientLight_64x.png");

    AmbientLightComponent->AttachToComponent(RootComponent);

}

AAmbientLight::~AAmbientLight()
{
}
