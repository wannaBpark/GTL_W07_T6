
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);

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

#include "Light.hlsl"

float LinearToSRGB(float val)
{
    float low  = 12.92 * val;
    float high = 1.055 * pow(val, 1.0 / 2.4) - 0.055;
    // linear가 임계값보다 큰지 판별 후 선형 보간
    float t = step(0.0031308, val); // linear >= 0.0031308이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float3 LinearToSRGB(float3 color)
{
    color.r = LinearToSRGB(color.r);
    color.g = LinearToSRGB(color.g);
    color.b = LinearToSRGB(color.b);
    return color;
}

float SRGBToLinear(float val)
{
    float low  = val / 12.92;
    float high = pow((val + 0.055) / 1.055, 2.4);
    float t = step(0.04045, val); // srgb가 0.04045 이상이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float3 SRGBToLinear(float3 color)
{
    color.r = SRGBToLinear(color.r);
    color.g = SRGBToLinear(color.g);
    color.b = SRGBToLinear(color.b);
    return color;
}

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

    // Diffuse
    float3 DiffuseColor = Material.DiffuseColor;
    if (Material.TextureFlag & (1 << 1))
    {
        DiffuseColor = DiffuseTexture.Sample(DiffuseSampler, Input.UV).rgb;
        DiffuseColor = SRGBToLinear(DiffuseColor);
    }

    // Normal
    float3 WorldNormal = Input.WorldNormal;
    if (Material.TextureFlag & (1 << 2))
    {
        float3 Normal = NormalTexture.Sample(NormalSampler, Input.UV).rgb;
        Normal = normalize(2.f * Normal - 1.f);
        WorldNormal = normalize(mul(mul(Normal, Input.TBN), (float3x3) InverseTransposedWorld));
    }
    
    // Lighting
    if (IsLit)
    {
#ifdef LIGHTING_MODEL_GOURAUD
        FinalColor = float4(Input.Color.rgb, 1.0);
#else
        float3 LightRgb = Lighting(Input.WorldPosition, WorldNormal, Input.WorldViewPosition).rgb;
        float3 LitColor = DiffuseColor * LightRgb;
        FinalColor = float4(LitColor, 1);
#endif
    }
    else
    {
        FinalColor = float4(DiffuseColor, 1);
    }
    
    if (bIsSelected)
    {
        FinalColor += float4(0.01, 0.01, 0.0, 1);
    }
    
    return FinalColor;
}
