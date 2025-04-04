#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Container/Array.h"
#include "World/WorldContext.h"

class UEngine : public UObject
{
    DECLARE_CLASS(UEngine, UObject)

public:
    UEngine() = default;

protected:
    TArray<FWorldContext> WorldList;
    int32 NextWorldContextHandle = 0;

public:
    virtual void Init();
    virtual void Tick(float DeltaTime) = 0;

    // TODO: UObject->GetWorld() 구현 이후 추가.
    UWorld* GetWorldFromContextObject(const UObject* Object) const;
    FWorldContext* GetWorldContextFromWorld(UWorld* InWorld);

    FWorldContext& CreateNewWorldContext(EWorldType InWorldType);
};

extern class UEngine* GEngine;