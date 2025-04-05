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
    WorldList.Add(EditorWorldContext);

    EditorWorld.reset(UWorld::CreateWorld(EWorldType::Editor, TEXT("EditorWorld")));
    EditorWorld->InitializeNewWorld();

    EditorWorldContext.SetCurrentWorld(EditorWorld.get());

}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext& WorldContext : WorldList)
    {
        UWorld* EditorWorld = WorldContext.World();
        if (EditorWorld && WorldContext.WorldType == EWorldType::Editor)
        {
            std::shared_ptr<ULevel> Level = EditorWorld->GetActiveLevel().lock();
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
        else if (WorldContext.WorldType == EWorldType::PIE)
        {
            UWorld* PIEWorld = WorldContext.World();
            if (PIEWorld)
            {
                std::shared_ptr<ULevel> Level = PIEWorld->GetActiveLevel().lock();
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
    for (FWorldContext& WorldContext : WorldList)
    {
        if (WorldContext.WorldType == EWorldType::Editor)
        {
            return WorldContext;
        }
    }
    return CreateNewWorldContext(EWorldType::Editor);
}

FWorldContext* UEditorEngine::GetPIEWorldContext(/*int32 WorldPIEInstance*/)
{
    for (FWorldContext& WorldContext : WorldList)
    {
        if (WorldContext.WorldType == EWorldType::PIE)
        {
            return &WorldContext;
        }
    }
    return nullptr;
}
