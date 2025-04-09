#include "FireballActor.h"
#include "Components/PointLightComponent.h"
AFireballActor::AFireballActor()
{
}

AFireballActor::~AFireballActor()
{
}

void AFireballActor::BeginPlay()
{
    UPointLightComponent* PointLight = GetComponentByClass<UPointLightComponent>();
    PointLight->SetAttenuation(0.5f);
    PointLight->SetSpecularColor(FLinearColor::Red);
    PointLight->SetDiffuseColor(FLinearColor::Red);
}
