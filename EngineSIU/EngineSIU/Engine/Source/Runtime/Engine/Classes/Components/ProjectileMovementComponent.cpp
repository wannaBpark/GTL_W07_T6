#include "ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"

UProjectileMovementComponent::UProjectileMovementComponent()
{
    InitialSpeed = 0.0f;
    MaxSpeed = 0.0f;
    ProjectileGravityScale = 1.0f;
    Velocity = FVector(0.01f,0,0);
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

void UProjectileMovementComponent::BeginPlay()
{
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    if (GetOwner())
    {
        FVector NewLocation = GetOwner()->GetRootComponent()->GetWorldLocation() + Velocity * DeltaTime;
        GetOwner()->GetRootComponent()->SetRelativeLocation(NewLocation);
    }
}

void UProjectileMovementComponent::OnComponentDestroyed()
{
}

