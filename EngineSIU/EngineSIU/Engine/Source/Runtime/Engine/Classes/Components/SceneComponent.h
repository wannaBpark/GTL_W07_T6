#pragma once
#include "ActorComponent.h"
#include "Math/Quat.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    virtual FVector GetForwardVector();
    virtual FVector GetRightVector();
    virtual FVector GetUpVector();
    void AddLocation(FVector _added);
    void AddRotation(FVector _added);
    void AddScale(FVector _added);

    USceneComponent* GetAttachParent() const { return AttachParent; }
    const TArray<USceneComponent*>& GetAttachChildren() const { return AttachChildren; }

    void AttachToComponent(USceneComponent* InParent);

public:
    FVector GetWorldLocation() const;
    FVector GetWorldRotation();
    FVector GetWorldScale() const;

    FVector GetRelativeLocation() const { return RelativeLocation; }
    void SetRelativeLocation(FVector NewLocation) { RelativeLocation = NewLocation; }

    FVector GetRelativeRotation() const;
    FQuat GetQuat() const { return QuatRotation; }
    void SetRelativeRotation(FVector NewRotation);
    void SetRelativeRotation(FQuat NewQuat) { QuatRotation = NewQuat; }

    FVector GetRelativeScale3D() const { return RelativeScale3D; }
    void SetRelativeScale3D(FVector NewScale) { RelativeScale3D = NewScale; }


    void SetupAttachment(USceneComponent* InParent);

protected:
    /** 부모 컴포넌트로부터 상대적인 위치 */
    UPROPERTY
    (FVector, RelativeLocation);

    /** 부모 컴포넌트로부터 상대적인 회전 */
    UPROPERTY
    (FVector, RelativeRotation);

    /** 부모 컴포넌트로부터 상대적인 회전(쿼터니언) */
    UPROPERTY
    (FQuat, QuatRotation);

    /** 부모 컴포넌트로부터 상대적인 크기 */
    UPROPERTY
    (FVector, RelativeScale3D);


    UPROPERTY
    (USceneComponent*, AttachParent, = nullptr);

    UPROPERTY
    (TArray<USceneComponent*>, AttachChildren);
};
