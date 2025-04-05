// light.hlsl

#define MAX_LIGHTS 16 

struct LIGHT
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular;

    float3 m_vPosition;
    float m_fFalloff;

    float3 m_vDirection;
    float pad1;

    float3 m_vAttenuation;
    float pad2;
    
    bool m_bEnable;
    float m_fRange;
    float padding;
    float pad3;
};


cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
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
        
        // 확산 조명 계산
        float fDiffuseFactor = dot(vToLight, vNormal);
        if (fDiffuseFactor > 0.0f)
        {
            if (Material.SpecularColor.a != 0.0f)
            {
                // 하프 벡터 대신 고정 벡터 사용(향후 수정 가능)
                float3 vHalf = float3(0.0f, 1.0f, 0.0f);
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), Material.SpecularColor.a);
            }
        }
        
        float fSpotFactor = pow(max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f), gLights[nIndex].m_fFalloff);
        
        // 거리 감쇠 계산
        float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        
        // 최종 조명 색상 반환
        return ((gLights[nIndex].m_cAmbient * Material.AmbientColor) +
                (gLights[nIndex].m_cDiffuse * fDiffuseFactor * Material.DiffuseColor) +
                (gLights[nIndex].m_cSpecular * fSpecularFactor * Material.SpecularColor))
                * fAttenuationFactor * fSpotFactor;
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    // 카메라 방향 계산
    
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            cColor += SpotLight(i, vPosition, vNormal);
        }
    }
    
    // 전역 환경광 추가
    cColor += (gcGlobalAmbientLight * Material.AmbientColor);
    cColor.a = Material.DiffuseColor.a;
    
    return cColor;
}
