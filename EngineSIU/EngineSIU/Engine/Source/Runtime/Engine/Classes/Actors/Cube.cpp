#include "Cube.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/FLoaderOBJ.h"

#include "GameFramework/Actor.h"

ACube::ACube()
{
    FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    

    //SetActorLocation(GetActorLocation() + FVector(1.0f, 0.0f, 0.0f));

}
