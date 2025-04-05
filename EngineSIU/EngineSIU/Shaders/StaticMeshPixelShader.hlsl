// staticMeshPixelShader.hlsl

Texture2D Textures : register(t0);
SamplerState Sampler : register(s0);

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 MVP;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

struct FMaterial
{
    float4 DiffuseColor;
    float TransparencyScalar;
    float4 AmbientColor;
    float DensityScalar;
    float4 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    float MaterialPad0;
};

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}


cbuffer FlagConstants : register(b3)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b4)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b5)
{
    float2 UVOffset;
    float2 TexturePad0;
}

#include "Light.hlsl"

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표,    
    float4 color : COLOR; // 전달할 색상      
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    bool normalFlag : TEXCOORD0; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
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
    
    // 텍스처 샘플링 및 기본 색상 계산
    float3 texColor = Textures.Sample(Sampler, input.texcoord + UVOffset);
    float3 color;
    if (texColor.g == 0)
    {
        color = saturate(Material.DiffuseColor);
    }
    else
    {
        color = texColor + Material.DiffuseColor.xyz;
    }
    
    // 선택된 객체에 하이라이트 적용
    if (isSelected)
    {
        color += float3(0.2, 0.2, 0.0); // 노란색 틴트
        if (IsSelectedSubMesh)
            color = float3(1, 1, 1);
    }
    
    // 조명 계산 및 발광 색상 추가
    if (IsLit)
    {
        if (input.normalFlag > 0.5)
        {
            color += Lighting(input.position.xyz, input.normal);
        }
        color += Material.EmissiveColor;
        
        output.color = float4(color, Material.TransparencyScalar);
        
        return output;
    }
    else // unlit 상태: PaperTexture 효과 적용
    {
        if (input.normalFlag < 0.5)
        {
            output.color = float4(color, Material.TransparencyScalar);
            return output;
        }
        output.color = float4(color, 1.0);
        
        output.color.a = Material.TransparencyScalar;
        
        return output;
    }
}
