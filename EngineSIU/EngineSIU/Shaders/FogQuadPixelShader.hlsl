Texture2D SceneColor : register(t0);
Texture2D Fog : register(t1);

Texture2D<float4> HeatMap : register(t2); // Added : To use heatmap texture

SamplerState Sampler : register(s0);

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

    float4 Scene = SceneColor.Sample(Sampler, UV);
    //float4 FogColor = Fog.Sample(Sampler, UV);
    float4 FogColor = HeatMap.Sample(Sampler, input.UV);
    FogColor.a = 0.5f;
    
    //float3 FinalColor = lerp(Scene.rgb, FogColor.rgb, FogColor.a);
    float3 FinalColor = FogColor.rgb;
    //float3 FinalColor = float3(1.0f, 0.0f, 0.0f);
    
    return float4(FinalColor.rgb, 1.0);
}
