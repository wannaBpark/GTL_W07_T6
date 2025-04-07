#define CAMERA_NEAR 0.1f
#define CAMERA_FAR  100.0f

Texture2D SceneColor : register(t126);
Texture2D SceneDepth : register(t127);
SamplerState Sampler : register(s0);

cbuffer ConstantBuffer : register(b1)
{
    row_major float4x4 InvViewProj;
    float4 FogColor;
    float3 CameraPos;
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float3 FogPosition;
    float padding;
};

cbuffer ScreenConstants : register(b0)
{
    float2 ScreenSize; // 전체 화면 크기 (w, h)
    float2 UVOffset; // 뷰포트 시작 UV (x/sw, y/sh)
    float2 UVScale; // 뷰포트 크기 비율 (w/sw, h/sh)
    float2 Padding;
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 UV = input.UV * UVScale + UVOffset;
    
    //픽셀 월드 위치 계산
    float depth = SceneDepth.Sample(Sampler, UV).r;
    depth = depth * 2.0 - 1.0f;
    
    float2 ndc;
    ndc.x = UV.x * 2.0 - 1.0;
    ndc.y = 1.0 - UV.y * 2.0;
    float4 clipPos = float4(ndc.x, ndc.y, depth, 1.0);
    float4 worldPos4 = mul(clipPos, InvViewProj);
    worldPos4 /= worldPos4.w;
    float3 worldPos = worldPos4.xyz;
    
    //높이 감쇠
    float HeightDiff = worldPos.z - FogPosition.z;
    float HeightFactor = exp(-HeightDiff * FogHeightFalloff);
    
     //카메라 까지의 거리 계산
    float3 ToCamera = CameraPos - worldPos;
    float distance = length(ToCamera);
    
    //안개 계수
    float FogFactor = 1.0 - exp(-distance / 2.0 * FogDensity * HeightFactor);
    if (HeightFactor == 0)
    {
        FogFactor = 1.0 - exp(-distance / 2.0 * FogDensity);
    }
    FogFactor = saturate(FogFactor);
    FogFactor = min(FogFactor, FogMaxOpacity);
    
    if (distance < StartDistance)
        FogFactor = 0;
    
    float4 SceneColorValue = SceneColor.Sample(Sampler, UV);
    float4 FinalColor = lerp(SceneColorValue, FogColor, FogFactor);
    return FinalColor;
}