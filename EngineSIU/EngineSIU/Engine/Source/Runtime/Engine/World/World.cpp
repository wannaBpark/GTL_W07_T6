#include "World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Actors/HeightFogActor.h"
#include "Actors/PointLightActor.h"
#include "Actors/SpotLightActor.h"
UWorld* UWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    UWorld* NewWorld = FObjectFactory::ConstructObject<UWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UWorld::InitializeNewWorld()
{
    ActiveLevel = FObjectFactory::ConstructObject<ULevel>(this);
    ActiveLevel->InitLevel(this);
}

void UWorld::SpawnLightGrid()
{
    static int currentHalfCountPerAxis = 3; // 처음엔 3 → 다음엔 4, 5...

    const int MinRadius = currentHalfCountPerAxis;
    const int MaxRadius = currentHalfCountPerAxis;

    const float Spacing = 300.0f;
    const float JitterAmount = 100.0f;
    const int MaxLights = 10000; // 총 한도 (원하면 제한)

    static int LightCount = 0;

    auto HashOffset = [JitterAmount](int x, int y, int z) -> FVector
        {
            auto LCG = [](int seed) -> float {
                seed = (1103515245 * seed + 12345) & 0x7fffffff;
                return (seed % 1000) / 1000.0f;
                };

            int seedBase = x * 73856093 ^ y * 19349663 ^ z * 83492791;
            float dx = (LCG(seedBase + 1) - 0.5f) * 2.0f * JitterAmount;
            float dy = (LCG(seedBase + 2) - 0.5f) * 2.0f * JitterAmount;
            float dz = (LCG(seedBase + 3) - 0.5f) * 2.0f * JitterAmount;
            return FVector(dx, dy, dz);
        };

    for (int x = -MaxRadius; x <= MaxRadius; ++x)
    {
        for (int y = -MaxRadius; y <= MaxRadius; ++y)
        {
            for (int z = -MaxRadius; z <= MaxRadius; ++z)
            {
                // 중심은 생략
                if (x == 0 && y == 0 && z == 0)
                    continue;

                // 이미 생성된 범위는 스킵
                if (FMath::Abs(x) < MinRadius &&
                    FMath::Abs(y) < MinRadius &&
                    FMath::Abs(z) < MinRadius)
                    continue;

                FVector basePos = FVector(x * Spacing, y * Spacing, z * Spacing);
                FVector jitter = HashOffset(x, y, z);
                FVector finalPos = basePos + jitter;

                if (LightCount % 2 == 0)
                {
                    APointLight* Light = SpawnActor<APointLight>();
                    Light->SetActorLabel(FString::Printf(TEXT("OBJ_PointLight_%d"), LightCount));
                    Light->SetActorLocation(finalPos);
                }
                else
                {
                    ASpotLight* Light = SpawnActor<ASpotLight>();
                    Light->SetActorLabel(FString::Printf(TEXT("OBJ_SpotLight_%d"), LightCount));
                    Light->SetActorLocation(finalPos);
                }

                if (++LightCount >= MaxLights)
                    return;
            }
        }
    }

    ++currentHalfCountPerAxis; // 다음 호출 땐 한 겹 더 퍼지게
}



UObject* UWorld::Duplicate(UObject* InOuter)
{
    // TODO: UWorld의 Duplicate는 역할 분리후 만드는것이 좋을듯
    UWorld* NewWorld = Cast<UWorld>(Super::Duplicate(InOuter));
    NewWorld->ActiveLevel = Cast<ULevel>(ActiveLevel->Duplicate(NewWorld));
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
        ActiveLevel = nullptr;
    }

    GUObjectArray.ProcessPendingDestroyObjects();
}

AActor* UWorld::SpawnActor(UClass* InClass)
{
    if (InClass->IsChildOf<AActor>())
    {
        AActor* NewActor = Cast<AActor>(FObjectFactory::ConstructObject(InClass, this));
        // TODO: 일단 AddComponent에서 Component마다 초기화
        // 추후에 RegisterComponent() 만들어지면 주석 해제
        // Actor->InitializeComponents();
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);
        return NewActor;
    }
    return nullptr;
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

UWorld* UWorld::GetWorld() const
{
    return const_cast<UWorld*>(this);
}
