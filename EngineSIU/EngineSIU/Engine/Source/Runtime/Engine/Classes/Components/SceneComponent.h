#pragma once
#include "ActorComponent.h"
#include "Math/Rotator.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();

    virtual UObject* Duplicate() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& InRayOrigin, FVector& InRayDirection, float& pfNearHitDistance);
    virtual FVector GetForwardVector();
    virtual FVector GetRightVector();
    virtual FVector GetUpVector();
    void AddLocation(FVector InAddValue);
    void AddRotation(FVector InAddValue);
    void AddScale(FVector InAddValue);

    USceneComponent* GetAttachParent() const { return AttachParent; }
    const TArray<USceneComponent*>& GetAttachChildren() const { return AttachChildren; }

    void AttachToComponent(USceneComponent* InParent);

protected:
    FVector RelativeLocation;
    FRotator RelativeRotation;
    FVector RelativeScale3D;

    USceneComponent* AttachParent = nullptr;
    TArray<USceneComponent*> AttachChildren;

public:
    FVector GetWorldScale() const;
    FRotator GetWorldRotation();
    FVector GetWorldLocation() const;
    
    FVector GetRelativeScale() const { return RelativeScale3D; }
    FRotator GetRelativeRotation() const { return RelativeRotation; }
    FVector GetRelativeLocation() const { return RelativeLocation; }

    FMatrix GetScaleMatrix() const;
    FMatrix GetRotationMatrix() const;
    FMatrix GetTranslationMatrix() const;

    FMatrix GetRTMatrix() const;
    FMatrix GetWorldMatrix() const;

    void SetLocation(FVector _newLoc) { RelativeLocation = _newLoc; }
    void SetRotation(FRotator InRotator) { RelativeRotation = InRotator; }
    void SetScale(FVector _newScale) { RelativeScale3D = _newScale; }
    void SetupAttachment(USceneComponent* InParent);
};
