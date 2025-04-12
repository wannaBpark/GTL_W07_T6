// light.hlsl

#define MAX_LIGHTS 16 

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
struct LIGHT
{
     float3 m_cDiffuse;
    float pad2;

    float3 m_cSpecular;
    float pad3;

    float3 m_vPosition;
    float m_fFalloff; // 스팟라이트의 감쇠 인자

    float3 m_vDirection;
    float pad4;

    float m_fAttenuation; // 거리 기반 감쇠 계수
    int m_bEnable;
    int m_nType;
    float m_fIntensity; // 광원 강도
    
    float m_fAttRadius; // 감쇠 반경 (Attenuation Radius)
    float3 LightPad;
    
    float m_fInnerCos; // cos(inner angle)
    float m_fOuterCos; // cos(outer angle)
    float2 Padding; // 정렬용
};

cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
    float3 padCB;
};

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float3 Direction;
    float Intensity;

    float4 DiffuseColor;
    float4 SpecularColor;
};

struct FPointLightInfo
{
    float3 Position;
    float Radius;

    float4 DiffuseColor;
    float4 SpecularColor;

    float Intensity;
    int Type;
    float2 Padding; // float[2]
};

struct FSpotLightInfo
{
    float3 Position;
    float Radius;

    float3 Direction;
    float pad3;

    float4 DiffuseColor;
    float4 SpecularColor;

    float Intensity;
    int Type;
    float InnerCos;
    float OuterCos;
};

cbuffer Lighting : register(b7)
{
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional[MAX_DIRECTIONAL_LIGHT];
    FPointLightInfo PointLights[MAX_POINT_LIGHT];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHT];
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    float pad0;
};

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    LIGHT light = gLights[nIndex];
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
    }
    
    float fSpotCos = dot(-vToLight, normalize(light.m_vDirection));
    float fSpotFactor = smoothstep(light.m_fOuterCos, light.m_fInnerCos, fSpotCos);

    if (fSpotFactor <= 0.0f)
        return float4(0, 0, 0, 0);
    //float fSpotFactor = pow(max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f), gLights[nIndex].m_fFalloff);
    float fAttenuationFactor = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
    
    float3 lit = (Ambient.AmbientColor * Material.AmbientColor.rgb) +
                 (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);

    // intensity와 attenuation factor, spot factor를 곱하여 최종 색상 계산
    return float4(lit * fAttenuationFactor * fSpotFactor * gLights[nIndex].m_fIntensity, 1.0f);
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
    }

    float fAttenuationFactor = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
   
    float3 lit = (Ambient.AmbientColor * Material.AmbientColor.rgb) +
                 (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);

    return float4(lit * fAttenuationFactor * gLights[nIndex].m_fIntensity, 1.0f);
}

float4 DirectionalLight(int nIndex, float3 vPosition, float3 vNormal)
{
    LIGHT light = gLights[nIndex];

    float3 vLightDir = normalize(-light.m_vDirection); // 빛의 방향 (표면 → 빛 방향)

    float fDiffuseFactor = saturate(dot(vNormal, vLightDir));

    float fSpecularFactor = 0.0f;
    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vLightDir + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1); // 또는 Material.Shininess
    }

    float3 lit = (Ambient.AmbientColor * Material.AmbientColor.rgb) +
                 (light.m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (light.m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);
    
    return float4(lit * light.m_fIntensity, 1.0f);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, vPosition, vNormal);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, vNormal);
            }
            else if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                cColor += DirectionalLight(i, vPosition, vNormal);
            }

        }
    }
    
    // 전역 환경광 추가
    cColor += Ambient.AmbientColor;
    cColor.a = 1;
    
    return cColor;
}





