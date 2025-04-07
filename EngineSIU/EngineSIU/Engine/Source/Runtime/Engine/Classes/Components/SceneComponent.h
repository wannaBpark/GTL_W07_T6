#pragma once
#include "ActorComponent.h"
#include "Math/Quat.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();

    virtual UObject* Duplicate() override;

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

protected:
    FVector RelativeLocation;
    FVector RelativeRotation;
    FQuat QuatRotation;
    FVector RelativeScale3D;

    USceneComponent* AttachParent = nullptr;
    TArray<USceneComponent*> AttachChildren;

public:
    virtual FVector GetWorldRotation();
    FVector GetWorldScale() const;
    FVector GetWorldLocation() const;
    FVector GetLocalRotation() const;
    FQuat GetQuat() const { return QuatRotation; }

    FVector GetLocalScale() const { return RelativeScale3D; }
    FVector GetLocalLocation() const { return RelativeLocation; }

    void SetLocation(FVector _newLoc) { RelativeLocation = _newLoc; }
    virtual void SetRotation(FVector _newRot);
    void SetRotation(FQuat _newRot) { QuatRotation = _newRot; }
    void SetScale(FVector _newScale) { RelativeScale3D = _newScale; }
    void SetupAttachment(USceneComponent* InParent);
};
