#include "ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"

UProjectileMovementComponent::UProjectileMovementComponent()
{
    InitialSpeed = 10.0f;
    MaxSpeed = 2000.0f;
    ProjectileGravityScale = 1.0f;
    Velocity = FVector(10.f, 0, 0);
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

void UProjectileMovementComponent::BeginPlay()
{
    FVector Forward = GetOwner()->GetActorForwardVector();
    Velocity = Forward * InitialSpeed;
}

void UProjectileMovementComponent::TickComponent(float dt)
{
    float DeltaTime = dt / 1000;

    Super::TickComponent(DeltaTime);

    const float Gravity = -0.f;
    Velocity.Z += Gravity * ProjectileGravityScale * DeltaTime;
    if (Velocity.Length() > MaxSpeed)
    {
        Velocity = Velocity.GetSafeNormal() * MaxSpeed;
    }
    if (GetOwner())
    {
        FVector NewLocation = GetOwner()->GetRootComponent()->GetWorldLocation() + Velocity * DeltaTime;
        GetOwner()->GetRootComponent()->SetRelativeLocation(NewLocation);
    }

    /*
    * TODO: Picking 됐을 때 Destroy 호출시
    * 
    *  USceneComponent::GetWorldLocation() const
	if (AttachParent)  예외 발생함.
    */

   /* 
      AccumulatedTime += DeltaTime;
    if (AccumulatedTime >= ProjectileLifetime)
      {
          if (GetOwner())
          {
              GetOwner()->Destroy();
          }
      }*/
}
