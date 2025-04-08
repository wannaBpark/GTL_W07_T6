#pragma once
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
class ALight :public AActor
{
    DECLARE_CLASS(ALight, AActor)
public:
    ALight();
    virtual ~ALight();
};