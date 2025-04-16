#include "LightGridGenerator.h"
#include "Actors/PointLightActor.h"
#include "Actors/SpotLightActor.h"
#include "World/World.h"

void FLightGridGenerator::GenerateLight(UWorld* world)
{
    const int MinRadius = currentHalfCountPerAxis;
    const int MaxRadius = currentHalfCountPerAxis;

    const float Spacing = 300.0f;
    const float JitterAmount = 100.0f;
    const int MaxLights = 10000; // 총 한도 (원하면 제한)

    static int LightCount = 0;

    auto HashOffset = [JitterAmount](int x, int y, int z) -> FVector
        {
            auto LCG = [](int seed) -> float {
                seed = (1103515245 * seed + 12345) & 0x7fffffff;
                return (seed % 1000) / 1000.0f;
                };

            int seedBase = x * 73856093 ^ y * 19349663 ^ z * 83492791;
            float dx = (LCG(seedBase + 1) - 0.5f) * 2.0f * JitterAmount;
            float dy = (LCG(seedBase + 2) - 0.5f) * 2.0f * JitterAmount;
            float dz = (LCG(seedBase + 3) - 0.5f) * 2.0f * JitterAmount;
            return FVector(dx, dy, dz);
        };

    for (int x = -MaxRadius; x <= MaxRadius; ++x)
    {
        for (int y = -MaxRadius; y <= MaxRadius; ++y)
        {
            for (int z = -MaxRadius; z <= MaxRadius; ++z)
            {
                // 중심은 생략
                if (x == 0 && y == 0 && z == 0)
                    continue;

                // 이미 생성된 범위는 스킵
                if (FMath::Abs(x) < MinRadius &&
                    FMath::Abs(y) < MinRadius &&
                    FMath::Abs(z) < MinRadius)
                    continue;

                FVector basePos = FVector(x * Spacing, y * Spacing, z * Spacing);
                FVector jitter = HashOffset(x, y, z);
                FVector finalPos = basePos + jitter;

                if (LightCount % 2 == 0)
                {
                    APointLight* Light = world->SpawnActor<APointLight>();
                    Light->SetActorLabel(FString::Printf(TEXT("OBJ_PointLight_%d"), LightCount));
                    Light->SetActorLocation(finalPos);
                }
                else
                {
                    ASpotLight* Light = world->SpawnActor<ASpotLight>();
                    Light->SetActorLabel(FString::Printf(TEXT("OBJ_SpotLight_%d"), LightCount));
                    Light->SetActorLocation(finalPos);
                }

                if (++LightCount >= MaxLights)
                    return;
            }
        }
    }

    ++currentHalfCountPerAxis; // 다음 호출 땐 한 겹 더 퍼지게
}

void FLightGridGenerator::DeleteLight(UWorld* world)
{
}
