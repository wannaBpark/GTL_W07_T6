#include "Level.h"
#include "GameFramework/Actor.h"
#include "UObject/Casts.h"


void ULevel::InitLevel(UWorld* InOwningWorld)
{
    OwningWorld = InOwningWorld;

}

UObject* ULevel::Duplicate()
{
    ThisClass* NewLevel = Cast<ThisClass>(Super::Duplicate());

    NewLevel->OwningWorld = OwningWorld;

    for (AActor* Actor : Actors)
    {
        NewLevel->Actors.Emplace(static_cast<AActor*>(Actor->Duplicate()));
    }

    return NewLevel;
}
