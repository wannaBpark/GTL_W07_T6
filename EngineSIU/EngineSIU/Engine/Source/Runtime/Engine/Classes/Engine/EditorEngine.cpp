#include "EditorEngine.h"

#include "World/World.h"
#include "Level.h"
#include "GameFramework/Actor.h"

void UEditorEngine::Init()
{
    Super::Init();

    // Initialize the engine
    GEngine = this;

    FWorldContext& EditorWorldContext = CreateNewWorldContext(EWorldType::Editor);

    EditorWorld.reset(UWorld::CreateWorld(EWorldType::Editor, FString("EditorWorld")));
    EditorWorld->InitializeNewWorld();

    EditorWorldContext.SetCurrentWorld(EditorWorld.get());
    ActiveWorld = EditorWorld;
}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            if (UWorld* World = WorldContext->World())
            {
                // TODO: World에서 EditorPlayer 제거 후 Tick 호출 제거 필요.
                World->Tick(DeltaTime);
                std::shared_ptr<ULevel> Level = World->GetActiveLevel().lock();
                if (Level)
                {
                    for (AActor* Actor : Level->Actors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
        else if (WorldContext->WorldType == EWorldType::PIE)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                std::shared_ptr<ULevel> Level = World->GetActiveLevel().lock();
                if (Level)
                {
                    for (AActor* Actor : Level->Actors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
    }
}

void UEditorEngine::StartPIE()
{
    FWorldContext& PIEWorldContext = CreateNewWorldContext(EWorldType::PIE);
    // PIEWorld = Cast<UWorld>(EditorWorld->Duplicate());
    
    // PIEWorld->BeginPlay();
    // 여기서 Actor들의 BeginPlay를 해줄지 안에서 해줄 지 고민.
    // WorldList.Add(*GetWorldContextFromWorld(PIEWorld.get()));
    // ActiveWorld = PIEWorld;
}

void UEditorEngine::EndPIE()
{
    if (PIEWorld)
    {
        //WorldList.Remove(*GetWorldContextFromWorld(PIEWorld.get()));

        PIEWorld->Release();
        PIEWorld.reset();
    }

    // 다시 EditorWorld로 돌아옴.

}

FWorldContext& UEditorEngine::GetEditorWorldContext(/*bool bEnsureIsGWorld*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            return *WorldContext;
        }
    }
    return CreateNewWorldContext(EWorldType::Editor);
}

FWorldContext* UEditorEngine::GetPIEWorldContext(/*int32 WorldPIEInstance*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::PIE)
        {
            return WorldContext;
        }
    }
    return nullptr;
}
