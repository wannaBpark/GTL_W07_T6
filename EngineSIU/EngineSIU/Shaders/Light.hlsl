// light.hlsl

#define MAX_LIGHTS 16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2

struct LIGHT
{
    // Ambient (float3) + padding
    float3 m_cAmbient;
    float pad1;

    // Diffuse (float3) + padding
    float3 m_cDiffuse;
    float pad2;

    // Specular (float3) + padding
    float3 m_cSpecular;
    float pad3;

    // Position (float3) + Falloff
    float3 m_vPosition;
    float m_fFalloff;

    // Direction (float3) + padding
    float3 m_vDirection;
    float pad4;

    // Attenuation (float3) + padding
    float3 m_vAttenuation;
    float pad5;

    // Enable + Range + padding (총 16바이트)
    int m_bEnable;
    int m_nType;
    
    float m_fRange;
    float pad7;
};

cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
    float3 padCB;
};

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance <= gLights[nIndex].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance; // 정규화
        
        float fDiffuseFactor = saturate(dot(vToLight, vNormal));
        if (fDiffuseFactor > 0.0f)
        {
            float3 vHalf = float3(0.0f, 1.0f, 0.0f);
            fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), 1);
        }
        
        float fSpotFactor = pow(max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f), gLights[nIndex].m_fFalloff);
        
        float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        
        float3 lit = (gLights[nIndex].m_cAmbient.rgb * Material.AmbientColor.rgb) + (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) + (gLights[nIndex].m_cSpecular * fSpecularFactor * Material.SpecularColor);

        return float4(lit * fAttenuationFactor * fSpotFactor, 1.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    if (fDistance <= gLights[nIndex].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        float fDiffuseFactor = saturate(dot(vToLight, normalize(vNormal)));
        if (fDiffuseFactor > 0.0f)
        {
            float3 vHalf = float3(0.0f, 1.0f, 0.0f);
            fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
            
        }
        float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));

        
        float3 lit = (gLights[nIndex].m_cAmbient.rgb * Material.AmbientColor.rgb) + (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) + (gLights[nIndex].m_cSpecular * fSpecularFactor * Material.SpecularColor);

        return float4(lit * fAttenuationFactor, 1.0f);
    }
    return (float4(0.0f, 0.0f, 0.0f, 0.0f));
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
        }
    }
    
    // 전역 환경광 추가
    cColor += (gcGlobalAmbientLight * float4(Material.AmbientColor, 1));
    cColor.a = 1;
    
    return cColor;
}
