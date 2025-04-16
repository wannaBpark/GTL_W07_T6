#pragma once
#include "Launch/Define.h"
class UWorld;
class AActor;
class FLightGridGenerator
{
public:
    void GenerateLight(UWorld* world);
    void DeleteLight(UWorld* world);
private:
    TArray<TArray<AActor*>> LightGrid;
    int currentHalfCountPerAxis = 3;
};

