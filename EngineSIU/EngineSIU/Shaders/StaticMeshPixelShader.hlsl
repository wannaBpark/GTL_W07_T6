// staticMeshPixelShader.hlsl

Texture2D Textures : register(t0);
Texture2D NormalMap : register(t1);
SamplerState Sampler : register(s0);

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float pad;
};

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    
    float3 AmbientColor;    //do not use
    float DensityScalar;
    
    float3 SpecularColor;
    float SpecularScalar;
    
    float3 EmissiveColor;
    float MaterialPad0;
};
cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b4)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b5)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b6)
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

struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치
    float4 color : COLOR; // 전달된 베이스 컬러
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그
    float2 texcoord : TEXCOORD2; // UV 좌표
    float3 normal : TEXCOORD5;        //N
    float3x3 mTBN : TBN;
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
};

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;
    output.UUID = UUID;
    
    float3 albedo = Material.DiffuseColor.rgb;
    if (TextureFlags & 1 << 1)
    {
        albedo = Textures.Sample(Sampler, input.texcoord).rgb;
    }
    float3 baseColor = albedo;
    
    float3 normal = input.normal;
    if (TextureFlags & 1 << 2)
    {
        float3 normalTS = NormalMap.Sample(Sampler, input.texcoord).rgb;
        normalTS = normalize(normalTS * 2.0f - 1.0f);
        normal = normalize(mul(normalTS, input.mTBN));
    }
    
#ifdef LIGHTING_MODEL_GOURAUD
    if (IsLit)
    {
        float3 finalColor = input.color.rgb * baseColor.rgb;
        output.color = float4(finalColor, 1.0);
    }
    else
    {
        output.color = float4(baseColor, 1.0);
    }
#else
    if (IsLit)
    {
        float4 litColor = Lighting(input.worldPos, normalize(normal));
        float3 finalColor = litColor.rgb * baseColor.rgb;
        output.color = float4(finalColor, 1.0);
    }
    else
    {
        output.color = float4(baseColor, 1.0);
    }
#endif
    
    if (IsSelectedSubMesh)
    {
        output.color += float4(0.02, 0.02, 0.02, 0);
    }
    
    return output;
}
