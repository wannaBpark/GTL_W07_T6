#include "World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Actors/HeightFogActor.h"

UWorld* UWorld::CreateWorld(const EWorldType InWorldType, const FString& InWorldName)
{
    UWorld* NewWorld = FObjectFactory::ConstructObject<UWorld>();
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UWorld::InitializeNewWorld()
{
    ActiveLevel.reset(FObjectFactory::ConstructObject<ULevel>());
    ActiveLevel->InitLevel(this);

}

UObject* UWorld::Duplicate()
{
    // TODO: UWorld의 Duplicate는 역할 분리후 만드는것이 좋을듯
    UWorld* NewWorld = Cast<UWorld>(Super::Duplicate());
    NewWorld->ActiveLevel = std::shared_ptr<ULevel>(Cast<ULevel>(ActiveLevel->Duplicate()));
    NewWorld->ActiveLevel->InitLevel(NewWorld);
    
    
    return NewWorld;
}

void UWorld::Tick(float DeltaTime)
{
    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();
}

void UWorld::BeginPlay()
{
    for (AActor* Actor : ActiveLevel->Actors)
    {
        if (Actor->GetWorld() == this)
        {
            Actor->BeginPlay();
        }
    }
}

void UWorld::Release()
{
    if (ActiveLevel)
    {
	    for (AActor* Actor : ActiveLevel->Actors)
	    {
		    Actor->EndPlay(EEndPlayReason::WorldTransition);
            TSet<UActorComponent*> Components = Actor->GetComponents();
	        for (UActorComponent* Component : Components)
	        {
	            GUObjectArray.MarkRemoveObject(Component);
	        }
	        GUObjectArray.MarkRemoveObject(Actor);
	    }
        ActiveLevel->Actors.Empty();
        ActiveLevel.reset();
    }

    GUObjectArray.ProcessPendingDestroyObjects();
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TSet<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    ActiveLevel->Actors.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}
