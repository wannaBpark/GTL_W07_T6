#include "EditorEngine.h"
#include "World/World.h"

void UEditorEngine::Init()
{
    Super::Init();
}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext& WorldContext : WorldList)
    {
        UWorld* EditorWorld = WorldContext.World();
        if (EditorWorld && WorldContext.WorldType == EWorldType::Editor)
        {
            
        }
    }
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
