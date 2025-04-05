#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class AActor;
class UWorld;

class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)

public:
    ULevel() = default;

    UWorld* OwningWorld;
    TArray<AActor*> Actors;
};
