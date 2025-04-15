
#include "ShaderRegisters.hlsl"

Texture2D SceneDepthTexture : register(t99);

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer ViewportSizeBuffer : register(b2)
{
    float2 ViewportSize;
    float2 Padding;
}

float4 mainPS(PS_INPUT_StaticMesh input)
{
    float4 FinalColor = float4(Material.DiffuseColor, 1);
    if (bIsSelected)
    {
        FinalColor += float4(0.5f, 0.5f, 0.5f, 1); // 선택된 경우 빨간색으로 설정
    }
      
    return FinalColor;
}
