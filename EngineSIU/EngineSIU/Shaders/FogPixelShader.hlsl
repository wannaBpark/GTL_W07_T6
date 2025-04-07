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
    float2 UV = input.UV;
    //float2 UV = input.UV * UVScale + UVOffset;
    
    //픽셀 월드 위치 계산
    float depth = SceneDepth.Sample(Sampler, UV).r;
    float z = depth * 2.0 - 1.0;
    //z = (2.0 * CAMERA_NEAR * CAMERA_FAR) / (CAMERA_FAR + CAMERA_NEAR - z * (CAMERA_FAR - CAMERA_NEAR));
    
    float2 ndc = UV * 2.0 - 1.0;
    float4 clipPos = float4(ndc.x, ndc.y, z, 1.0);
    float4 worldPos4 = mul(clipPos, InvViewProj);
    worldPos4 /= worldPos4.w;
    float3 worldPos = worldPos4.xyz;
    
    //카메라 까지의 거리 계산
    float3 ToCamera = CameraPos - worldPos;
    float distance = length(ToCamera);
    if (distance < StartDistance)
        distance = 0;
    
    //높이 감쇠
    float HeightDiff = (worldPos.z)/100.0;
    float HeightFactor = exp(-HeightDiff * FogHeightFalloff);
    
    //안개 계수
    float FogFactor = FogDensity * HeightFactor;
    FogFactor = min(FogFactor, FogMaxOpacity);
    FogFactor = saturate(FogFactor);
    
    float4 SceneColorValue = SceneColor.Sample(Sampler, input.UV);
    float4 FinalColor = lerp(SceneColorValue, FogColor, FogFactor);
    return FinalColor;
    // 1) 뷰포트별 UV 계산
    //float2 uv = input.UV * UVScale + UVOffset;
    //
    //// 2) 원시 깊이 샘플링
    //float raw = SceneDepth.Sample(Sampler, uv).r;
    //
    //// 3) NDC → 선형 깊이 변환
    //float z_ndc = raw * 2.0 - 1.0;
    //float lin = (2.0 * CAMERA_NEAR * CAMERA_FAR)
    //              / (CAMERA_FAR + CAMERA_NEAR - z_ndc * (CAMERA_FAR - CAMERA_NEAR));
    //
    //// 4) 0~1 정규화
    //float norm = saturate((lin - CAMERA_NEAR) / (CAMERA_FAR - CAMERA_NEAR));
    //
    //return float4(norm, norm, norm, 1.0);
}