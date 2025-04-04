Texture2D SceneColor : register(t126);
Texture2D SceneDepth : register(t127);
SamplerState Sampler : register(s0);

cbuffer ConstantBuffer : register(b6)
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

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.Position = float4(input.Position, 1.0f);
    output.UV = input.UV;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float4 FinalColor = float4(1, 0, 0, 1);
    
    return FinalColor;
}