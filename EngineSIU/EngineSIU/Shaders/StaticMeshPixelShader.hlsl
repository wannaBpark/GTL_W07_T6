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
    
    float3 AmbientColor;
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

#ifndef LIGHTING_MODEL_GOURAUD
#include "Light.hlsl"
#endif

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
    
#ifdef LIGHTING_MODEL_GOURAUD
    if (IsLit)
    {
        float3 finalColor = input.color.rgb * baseColor;
        output.color = float4(finalColor, 1.0);
    }
    else
    {
        // Unlit 모드
        output.color = float4(baseColor, 1.0);
    }
#elif defined(LIGHTING_MODEL_LAMBERT)
    // Lambert: 픽셀 셰이더에서 간단한 조명 모델 계산
    if (IsLit && input.normalFlag > 0.5)
    {
        float3 N = normalize(input.normal);
        float3 litColor = float3(0, 0, 0);
        
        // 모든 활성화된 광원에 대해 Lambert 조명 계산
        for (int i = 0; i < gnLights; i++)
        {
            if (gLights[i].m_bEnable)
            {
                // 광원 위치에서 표면까지의 벡터 (정규화)
                float3 lightDir;
                float attenuation = 1.0;
                
                if (gLights[i].m_nType == POINT_LIGHT || gLights[i].m_nType == SPOT_LIGHT)
                {
                    float3 lightVec = gLights[i].m_vPosition - input.worldPos;
                    float distance = length(lightVec);
                    
                    // 감쇠 반경 체크
                    if (distance > gLights[i].m_fAttRadius)
                        continue;
                        
                    lightDir = normalize(lightVec);
                    attenuation = 1.0 / (1.0 + gLights[i].m_fAttenuation * distance * distance);
                    
                    // 스팟라이트인 경우 추가 감쇠 계산
                    if (gLights[i].m_nType == SPOT_LIGHT)
                    {
                        float spotFactor = pow(max(dot(-lightDir, gLights[i].m_vDirection), 0.0), gLights[i].m_fFalloff);
                        attenuation *= spotFactor;
                    }
                }
                else // Directional light
                {
                    lightDir = normalize(-gLights[i].m_vDirection);
                }
                
                // Lambert 조명 모델: N·L
                float NdotL = max(dot(N, lightDir), 0.0);
                float3 diffuse = gLights[i].m_cDiffuse * baseColor * NdotL;
                
                // 최종 조명 색상에 기여도 추가
                litColor += diffuse * attenuation * gLights[i].m_fIntensity;
            }
        }
        
        // 환경광 추가
        litColor += gcGlobalAmbientLight.rgb * Material.AmbientColor;
        
        output.color = float4(litColor, 1.0);
    }
    else
    {
        // Unlit 모드 또는 노말이 없는 경우
        output.color = float4(baseColor, 1.0);
    }
#else
    if (IsLit && input.normalFlag > 0.5)
    {
        // 기존 Light.hlsl의 Lighting 함수 활용
        float4 litColor = Lighting(input.worldPos, normalize(input.normal));
        float3 finalColor = baseColor * litColor.rgb;
        output.color = float4(finalColor, 1.0);
    }
    else
    {
        // Unlit 모드 또는 노말이 없는 경우
        output.color = float4(baseColor, 1.0);
    }
#endif
    
    if (IsSelectedSubMesh)
    {
        output.color += float4(0.02, 0.02, 0.02, 0);
    }
    
    //output.color = float4(input.normal, 1);

    return output;
}
