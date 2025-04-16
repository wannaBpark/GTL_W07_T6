
#include "ShaderRegisters.hlsl"

SamplerState Sampler : register(s0);

Texture2D Textures : register(t0);
Texture2D NormalMap : register(t1);

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b3)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b4)
{
    float2 UVOffset;
    float2 TexturePad0;
}

cbuffer TextureFlagConstants : register(b7)
{
    uint TextureFlags;
    float3 TextureFlagPad;
}

#include "Light.hlsl"

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);
    
    float3 Albedo = Textures.Sample(Sampler, Input.UV).rgb;
    float3 MatDiffuse = Material.DiffuseColor.rgb;

    bool bHasTexture = any(Albedo != float3(0, 0, 0));
    
    float3 baseColor = bHasTexture ? Albedo : MatDiffuse;

    if (IsLit)
    {
        float3 LightRgb = Lighting(Input.WorldPosition, Input.WorldNormal).rgb;
        float3 LitColor = baseColor * LightRgb;
        FinalColor = float4(LitColor, 1);
    }
    else
    {
        FinalColor = float4(baseColor, 1);
    }
    
    if (bIsSelected)
    {
        FinalColor += float4(0.02, 0.02, 0.02, 1);
    }
    
    return FinalColor;
}
