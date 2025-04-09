#pragma once
#include"GameFramework/Actor.h"
class AFireballActor : public AActor
{
    DECLARE_CLASS(AFireballActor, AActor)
public:
    AFireballActor();
    virtual ~AFireballActor();
    virtual void BeginPlay() override;
};