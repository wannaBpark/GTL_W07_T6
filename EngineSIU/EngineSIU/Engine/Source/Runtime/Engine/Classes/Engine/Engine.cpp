#include "Engine.h"

UEngine* GEngine = nullptr;

FWorldContext* UEngine::GetWorldContextFromWorld(UWorld* InWorld)
{
    for (FWorldContext& WorldContext : WorldList)
    {
        if (WorldContext.World() == InWorld)
        {
            return &WorldContext;
        }
    }
    return nullptr;
}

FWorldContext& UEngine::CreateNewWorldContext(EWorldType InWorldType)
{
    FWorldContext* NewWorldContext = new FWorldContext();
    WorldList.Add(*NewWorldContext);
    NewWorldContext->WorldType = InWorldType;
    NewWorldContext->ContextHandle = FName(*FString::Printf(TEXT("WorldContext_%d"), NextWorldContextHandle++));

    return *NewWorldContext;
}
