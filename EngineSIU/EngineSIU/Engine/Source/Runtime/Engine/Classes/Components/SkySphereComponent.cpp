#include "SkySphereComponent.h"
#include "World.h"


USkySphereComponent::USkySphereComponent()
{
    SetType(StaticClass()->GetName());
}

void USkySphereComponent::TickComponent(float DeltaTime)
{
    UOffset += 0.005f;
    VOffset += 0.005f;
    Super::TickComponent(DeltaTime);
}
