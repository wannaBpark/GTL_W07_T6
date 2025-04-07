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
    float depth = 1; //SceneDepth.Sample(Sampler, input.UV).r;
    float2 ndc = input.UV * 2.0 - 1.0;
    float4 clipPos = float4(ndc, depth, 1.0);
    float4 worldPos4 = mul(clipPos, InvViewProj);
    worldPos4 /= worldPos4.w;
    float3 worldPos = worldPos4.xyz;
    
    float4 SceneColorValue = SceneColor.Sample(Sampler, input.UV);
    return SceneColorValue;// * float4(worldPos, 1.01);
}