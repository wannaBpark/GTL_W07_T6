#include "HeightFogComponent.h"
#include <UObject/Casts.h>

UHeightFogComponent::UHeightFogComponent(float Density, float HeightFalloff, float StartDist, float CutoffDist, float MaxOpacity)
    :FogDensity(Density), FogHeightFalloff(HeightFalloff), StartDistance(StartDist), FogCutoffDistance(CutoffDist), FogMaxOpacity(MaxOpacity)
{
    FogInscatteringColor = FLinearColor::White;
}

void UHeightFogComponent::SetFogDensity(float value)
{
    FogDensity = value;
}

void UHeightFogComponent::SetFogHeightFalloff(float value)
{
    FogHeightFalloff = value; 
}

void UHeightFogComponent::SetStartDistance(float value)
{
    StartDistance = value;
}

void UHeightFogComponent::SetFogCutoffDistance(float value)
{
    FogCutoffDistance = value;
}

void UHeightFogComponent::SetFogMaxOpacity(float value)
{
    FogMaxOpacity = value;
}

void UHeightFogComponent::SetFogColor(FLinearColor color)
{
    FogInscatteringColor = color;
}

UObject* UHeightFogComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->FogDensity = FogDensity;
    NewComponent->FogHeightFalloff = FogHeightFalloff;
    NewComponent->StartDistance = StartDistance;
    NewComponent->FogCutoffDistance = FogCutoffDistance;
    NewComponent->FogMaxOpacity = FogMaxOpacity;
    NewComponent->FogInscatteringColor = FogInscatteringColor;

    return NewComponent;
}
