#pragma once
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16

struct FAmbientLightInfo
{
    FVector4 AmbientColor;         // RGB + alpha
};

struct FDirectionalLightInfo
{
    FVector Direction;   // 정규화된 광선 방향 (월드 공간 기준)
    float   Intensity;   // 밝기

    FVector4 DiffuseColor;         // RGB + alpha
    FVector4 SpecularColor;         // RGB + alpha
};

struct FPointLightInfo
{
    FVector Position;    // 월드 공간 위치
    float   Radius;      // 감쇠가 0이 되는 거리

    FVector4 DiffuseColor;         // RGB + alpha
    FVector4 SpecularColor;         // RGB + alpha

    float   Intensity;   // 밝기
    int     Type;        // 라이트 타입 구분용 (예: 1 = Point)
    float   Padding[2];  // 16바이트 정렬
};

struct FSpotLightInfo
{
    FVector Position;       // 월드 공간 위치
    float   Radius;         // 감쇠 거리

    FVector Direction;      // 빛이 향하는 방향 (normalize)
    float   pad3;

    FVector4 DiffuseColor;         // RGB + alpha
    FVector4 SpecularColor;         // RGB + alpha

    float   Intensity;      // 밝기
    int     Type;           // 라이트 타입 구분용 (예: 2 = Spot)
    float   InnerCos; // cos(inner angle)
    float   OuterCos; // cos(outer angle)
};

struct FLightInfoBuffer
{
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional[MAX_DIRECTIONAL_LIGHT];
    FPointLightInfo PointLights[MAX_POINT_LIGHT];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHT];
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    float    pad0;
};
