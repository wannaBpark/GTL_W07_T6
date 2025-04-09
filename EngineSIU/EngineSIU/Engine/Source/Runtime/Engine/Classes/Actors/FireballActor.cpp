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
    GetComponentByClass<UPointLightComponent>()->SetAttenuation(0.5f);
    GetComponentByClass<UPointLightComponent>()->SetSpecularColor(FLinearColor::Red);
    GetComponentByClass<UPointLightComponent>()->SetDiffuseColor(FLinearColor::Red);
}
