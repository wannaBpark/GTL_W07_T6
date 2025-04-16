#pragma once
#include "Launch/Define.h"
class UWorld;
class AActor;


namespace ELightGridGenerator
{
    enum Mode
    {
        Generate,
        Delete,
        Reset
    };
}

class FLightGridGenerator
{
public:
    FLightGridGenerator();
    ~FLightGridGenerator();
    void GenerateLight(UWorld* world);
    void DeleteLight(UWorld* world);
    void Reset(UWorld* world);
private:
    TArray<TArray<AActor*>> LightGrid;
    int currentHalfCountPerAxis = startCountPerAxis;
    const int startCountPerAxis = 0;
    const float spacing = 300.0f;                       //Light간의 간격
    const float jitterAmount = 80.0f;                   //Light 위치 랜덤 값. 매번 같은 랜덤.
    FVector GetJitteredPosition(int x, int y, int z, float spacing, float jitterAmount);
};

