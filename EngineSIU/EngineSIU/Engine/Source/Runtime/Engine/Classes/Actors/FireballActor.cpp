#include "FireballActor.h"
#include "Engine/FLoaderOBJ.h"

#include "Components/PointLightComponent.h"

#include "Components/ProjectileMovementComponent.h"

#include "Components/SphereComp.h"

AFireballActor::AFireballActor()
{
    FManagerOBJ::CreateStaticMesh("Contents/Sphere.obj");


    SphereComp = AddComponent<USphereComp>();
    
    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));
  
    PointLightComponent = AddComponent<UPointLightComponent>();
    

    PointLightComponent->SetAttenuation(0.5f);
    
    PointLightComponent->SetSpecularColor(FLinearColor::Red);
    
    PointLightComponent->SetDiffuseColor(FLinearColor::Red);
    
    ProjectileMovementComponent = AddComponent<UProjectileMovementComponent>();
    PointLightComponent->AttachToComponent(RootComponent);
}

AFireballActor::~AFireballActor()
{
}

void AFireballActor::BeginPlay()
{
}
