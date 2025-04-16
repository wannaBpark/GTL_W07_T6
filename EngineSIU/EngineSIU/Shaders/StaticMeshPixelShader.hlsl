// staticMeshPixelShader.hlsl



Texture2D Textures : register(t0);
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



#include "Light.hlsl"



struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치
    float4 color : COLOR; // 전달된 베이스 컬러
    float3 normal : NORMAL; // 월드 공간 노멀
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그
    float2 texcoord : TEXCOORD2; // UV 좌표
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
    
    // 1) 알베도 샘플링
    float3 albedo = Textures.Sample(Sampler, input.texcoord).rgb;
    // 2) 머티리얼 디퓨즈
    float3 matDiffuse = Material.DiffuseColor.rgb;
    bool hasTexture = any(albedo != float3(0, 0, 0));
    
    float3 baseColor = hasTexture ? albedo : matDiffuse;
    
    // (1) 현재 픽셀이 속한 타일 계산 (input.position = 화면 픽셀좌표계)
    uint2 pixelCoord = uint2(input.position.xy);
    uint2 tileCoord = pixelCoord / TileSize; // 각 성분별 나눔
    uint tilesX = ScreenSize.x / TileSize.x; // 한 행에 존재하는 타일 수
    uint flatTileIndex = tileCoord.x + tileCoord.y * tilesX;
    
    // (2) 현재 타일의 조명 정보 읽기
    LightPerTiles tileLights = gLightPerTiles[flatTileIndex];
    
    // 조명 기여 누적 (예시: 단순히 조명 색상을 더함)
    float3 lightingAccum = 0;
    for (uint i = 0; i < tileLights.NumLights; ++i)
    {
         // tileLights.Indices[i] 는 전역 조명 인덱스
        uint globalLightIndex = tileLights.Indices[i];
        FPointLightInfo light = gPointLights[globalLightIndex];
        
        
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
    if (IsLit && input.normalFlag > 0.5)
    {
        float4 litColor = Lighting(input.worldPos, normalize(input.normal));
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
