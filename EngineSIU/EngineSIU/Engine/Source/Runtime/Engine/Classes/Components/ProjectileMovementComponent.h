#pragma once
#include"Components/SceneComponent.h"

class UProjectileMovementComponent : public USceneComponent
{
    DECLARE_CLASS(UProjectileMovementComponent, USceneComponent)
public:
    UProjectileMovementComponent();
    virtual ~UProjectileMovementComponent();
    void SetVelocity(FVector NewVelocity) { Velocity = NewVelocity; }
    FVector GetVelocity() const { return Velocity; }

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime) override;

private:
    float ProjectileLifetime = 10.0f; // 생명주기
    float AccumulatedTime = 0;

    float InitialSpeed;
    float MaxSpeed;
    
    float ProjectileGravityScale;
    FVector Velocity;
};

