#include "Level.h"
#include "UObject/Casts.h"


UObject* ULevel::Duplicate()
{
    ThisClass* NewLevel = Cast<ThisClass>(Super::Duplicate());

    NewLevel->OwningWorld = OwningWorld;

    for (AActor* Actor : Actors)
    {
        NewLevel->Actors.Emplace(Actor);
    }

    return NewLevel;
}
