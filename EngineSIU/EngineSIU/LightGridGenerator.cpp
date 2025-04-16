#include "LightGridGenerator.h"
#include "Actors/PointLightActor.h"
#include "Actors/SpotLightActor.h"
#include "World/World.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
FLightGridGenerator::FLightGridGenerator()
{
}

FLightGridGenerator::~FLightGridGenerator()
{
    for (TArray<AActor*>& shell : LightGrid)
    {
        for (AActor* actor : shell)
        {
            if (actor)
            {
                actor->Destroy();
            }
        }
        shell.Empty();
    }

    LightGrid.Empty();
}

void FLightGridGenerator::GenerateLight(UWorld* world)
{
    int minR = currentHalfCountPerAxis;
    int maxR = currentHalfCountPerAxis;

    int r = currentHalfCountPerAxis;
    int outerCube = (2 * r + 1) * (2 * r + 1) * (2 * r + 1);
    int innerCube = FMath::Max(0, (2 * r - 1) * (2 * r - 1) * (2 * r - 1));
    int approxShellSize = outerCube - innerCube;

    TArray<AActor*> newShell;
    newShell.Reserve(approxShellSize);


    for (int x = -maxR; x <= maxR; ++x)
    {
        for (int y = -maxR; y <= maxR; ++y)
        {
            for (int z = -maxR; z <= maxR; ++z)
            {
                //if (x == 0 && y == 0 && z == 0) continue;

                // 이전 쉘 내부는 제외
                if (FMath::Abs(x) < minR && FMath::Abs(y) < minR && FMath::Abs(z) < minR)
                    continue;

                FVector pos = GetJitteredPosition(x, y, z, spacing, jitterAmount);

                AActor* light = nullptr;
                if ((shellLightCount % 2) == 0)
                {
                    light = world->SpawnActor<APointLight>();
                    light->SetActorLabel(FString::Printf(TEXT("PointLight_%d"), shellLightCount));
                    UPointLightComponent* comp = light->GetComponentByClass<UPointLightComponent>();
                    //comp->SetLightColor(RandomColor());
                }
                else
                {
                    light = world->SpawnActor<ASpotLight>();
                    light->SetActorLabel(FString::Printf(TEXT("SpotLight_%d"), shellLightCount));
                    USpotLightComponent* comp = light->GetComponentByClass<USpotLightComponent>();
                    //comp->SetLightColor(RandomColor());
                }

                if (light)
                {
                    light->SetActorLocation(pos);
                    newShell.Add(light);
                }

                ++shellLightCount;
            }
        }
    }

    if (LightGrid.Num() <= currentHalfCountPerAxis)
    {
        LightGrid.SetNum(currentHalfCountPerAxis + 1);
    }

    LightGrid[currentHalfCountPerAxis] = std::move(newShell);
    ++currentHalfCountPerAxis;
}


void FLightGridGenerator::DeleteLight(UWorld* world)
{
    if (currentHalfCountPerAxis <= 0 || LightGrid.Num() == 0)
        return;

    int shellIndex = currentHalfCountPerAxis - 1;

    // 유효한 인덱스인지 확인
    if (!LightGrid.IsValidIndex(shellIndex))
        return;

    TArray<AActor*>& shell = LightGrid[shellIndex];

    for (AActor* light : shell)
    {
        if (light)
        {
            light->Destroy();
        }
    }

    // TArray는 SetNum을 통해 capacity 보존 가능
    shell.Empty();

    --currentHalfCountPerAxis;
}

void FLightGridGenerator::Reset(UWorld* world)
{
    for (TArray<AActor*>& shell : LightGrid)
    {
        for (AActor* light : shell)
        {
            if (light)
                light->Destroy();
        }
        shell.Empty();
    }
    currentHalfCountPerAxis = startCountPerAxis;
}

FVector FLightGridGenerator::GetJitteredPosition(int x, int y, int z, float spacing, float jitterAmount)
{
    auto LCG = [](int seed) -> float {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        return (seed % 1000) / 1000.0f;
        };

    int seedBase = x * 73856093 ^ y * 19349663 ^ z * 83492791;
    float dx = (LCG(seedBase + 1) - 0.5f) * 2.0f * jitterAmount;
    float dy = (LCG(seedBase + 2) - 0.5f) * 2.0f * jitterAmount;
    float dz = (LCG(seedBase + 3) - 0.5f) * 2.0f * jitterAmount;

    return FVector(x * spacing + dx, y * spacing + dy, z * spacing + dz);
}

FLinearColor FLightGridGenerator::RandomColor()
{
    auto HashFloat01 = [](int seed) -> float {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        return (seed % 1000) / 1000.0f; // 0.0 ~ 0.999
        };

    int colorSeed = shellLightCount * 1234567; // shellLightCount를 기반 시드로 사용

    float r = HashFloat01(colorSeed + 1);
    float g = HashFloat01(colorSeed + 2);
    float b = HashFloat01(colorSeed + 3);

    return FLinearColor(r,g,b,1.0f);
}

