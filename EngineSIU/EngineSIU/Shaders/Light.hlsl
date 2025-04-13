#define MAX_LIGHTS 16 

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3

struct LIGHT
{
    float3 m_cDiffuse; // 광원의 확산 색상
    float pad2;

    float3 m_cSpecular; // 광원의 반사 색상
    float pad3;

    float3 m_vPosition; // 광원의 위치 (Point, Spot)
    float m_fFalloff; // 스팟라이트의 감쇠 인자

    float3 m_vDirection; // 광원의 방향 (Spot, Directional)
    float pad4;

    float m_fAttenuation; // 거리 기반 감쇠 계수
    int m_bEnable; // 광원 활성화 여부
    int m_nType; // 광원 유형
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

float CalculateAttenuation(float distance, float attenuationFactor, float radius)
{
    if (distance > radius)
        return 0.0;
        
    return 1.0 / (1.0 + attenuationFactor * distance * distance);
}

float CalculateSpotEffect(float3 lightDir, float3 spotDir, float spotFalloff)
{
    float cosAngle = dot(-lightDir, spotDir);
    return pow(max(cosAngle, 0.0), spotFalloff);
}

float CalculateDiffuse(float3 normal, float3 lightDir)
{
    return max(dot(normal, lightDir), 0.0);
}

float CalculateSpecular(float3 normal, float3 lightDir, float3 viewDir, float specularPower)
{
    float NdotL = dot(normal, lightDir);
    if (NdotL <= 0.0)
        return 0.0;
        
    float3 halfVector = normalize(lightDir + viewDir);
    return pow(max(dot(normal, halfVector), 0.0), max(specularPower, 1.0) * 20.0);
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal)
{
#ifdef LIGHTING_MODEL_GOURAUD
    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 vToLight = light.m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.m_fAttRadius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor = max(dot(vNormal, lightDir), 0.0);
    float attenuation = 1.0 / (1.0 + light.m_fAttenuation * fDistance * fDistance);
    
    float specularFactor = 0.0;
    if (diffuseFactor > 0.0)
    {
        float3 viewDir = normalize(CameraPosition - vPosition);
        float3 halfVector = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(vNormal, halfVector), 0.0), 4.0);
    }
    
    float3 lit = (light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor) +
                 (light.m_cSpecular.rgb * specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuation * light.m_fIntensity, 1.0);
#elif defined(LIGHTING_MODEL_LAMBERT)

    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 vToLight = light.m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.m_fAttRadius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor = max(dot(vNormal, lightDir), 0.0);
    float attenuation = 1.0 / (1.0 + light.m_fAttenuation * fDistance * fDistance);
    
    // Lambert에서는 specular 계산 없음
    float3 lit = light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor;
                 
    return float4(lit * attenuation * light.m_fIntensity, 1.0);
    
#else
    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.f, 0.f, 0.f, 0.f);
    
    float3 vToLight = light.m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    float fAttenuation = CalculateAttenuation(fDistance, light.m_fAttenuation, light.m_fAttRadius);
    if (fAttenuation <= 0.0)
        return float4(0.f, 0.f, 0.f, 0.f);
    
    float3 lightDir = normalize(vToLight);
    
    float3 viewDir = normalize(CameraPosition - vPosition);
    
    float diffuseFactor = CalculateDiffuse(vNormal, lightDir);
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor) +
                 (light.m_cSpecular.rgb * specularFactor * Material.SpecularColor);
                 
    return float4(lit * fAttenuation * light.m_fIntensity, 1.0);
    
    //// 광원과 픽셀 위치 간 벡터 계산
    //float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    //float fDistance = length(vToLight);

    //// 감쇠 반경을 벗어나면 기여하지 않음
    //if (fDistance > gLights[nIndex].m_fAttRadius)
    //{
    //    return float4(0.0f, 0.0f, 0.0f, 1.0f);
    //}
    
    //float fSpecularFactor = 0.0f;
    //vToLight /= fDistance; // 정규화
    //float fDiffuseFactor = max(dot(vNormal, vToLight), 0.0f);
    ////float fDiffuseFactor = max(dot(float3(1, 0, 0), vToLight), 0.0f);
    
    //if (fDiffuseFactor > 0.0f)
    //{
    //    float3 vToEye = normalize(CameraPosition - vPosition);
    //    float3 vHalf = normalize(vToLight + vToEye);
    //    fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 20);
        
    //    //return float4(1, 0, 0, 1);
    //}

    //float fAttenuationFactor = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
   
    //float3 lit = /*(gcGlobalAmbientLight * Material.AmbientColor.rgb) +*/
    //             (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
    //             (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);
       
    ////float3 lit = (gcGlobalAmbientLight * Material.AmbientColor.rgb) +
    ////             (gLights[nIndex].m_cDiffuse.rgb * 1 * Material.DiffuseColor) +
    ////             (gLights[nIndex].m_cSpecular.rgb * 1 * Material.SpecularColor);
    
    ////return float4(vNormal, 1);
    
    ////return float4(fDiffuseFactor * 100, fDiffuseFactor * 100, fDiffuseFactor * 100, 1);
    
    //return float4(lit * fAttenuationFactor * gLights[nIndex].m_fIntensity, 1.0f);
    
#endif
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
#ifdef LIGHTING_MODEL_GOURAUD
    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 vToLight = light.m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.m_fAttRadius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor = max(dot(vNormal, lightDir), 0.0);
    float attenuation = 1.0 / (1.0 + light.m_fAttenuation * fDistance * fDistance);
    float spotFactor = pow(max(dot(-lightDir, normalize(light.m_vDirection)), 0.0), light.m_fFalloff);
    
    // 간소화된 specular 계산
    float specularFactor = 0.0;
    if (diffuseFactor > 0.0)
    {
        float3 viewDir = normalize(CameraPosition - vPosition);
        float3 halfVector = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(vNormal, halfVector), 0.0), 4.0);
    }
    
    float3 lit = (light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor) +
                 (light.m_cSpecular.rgb * specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuation * spotFactor * light.m_fIntensity, 1.0);
#elif defined(LIGHTING_MODEL_LAMBERT)
    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 vToLight = light.m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    // Begin test
    //if (fDistance > light.m_fAttRadius)
    //    return float4(0.0, 0.0, 0.0, 0.0);
    
    //float3 lightDir = normalize(vToLight);
    //float diffuseFactor = max(dot(vNormal, lightDir), 0.0);
    //float attenuation = 1.0 / (1.0 + light.m_fAttenuation * fDistance * fDistance);
    //float spotFactor = pow(max(dot(-lightDir, normalize(light.m_vDirection)), 0.0), light.m_fFalloff);
    
    //// Lambert에서는 specular 계산 없음
    //float3 lit = light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor;
                 
    //return float4(lit * attenuation * spotFactor * light.m_fIntensity, 1.0);
    
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor = max(dot(vNormal, lightDir), 0.0);
    float spotFactor = pow(max(dot(-lightDir, normalize(light.m_vDirection)), 0.0), light.m_fFalloff);
    
    // Lambert에서는 specular 계산 없음
    float3 lit = light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor;
                 
    return float4(lit * spotFactor * light.m_fIntensity, 1.0);
    // End test
#else
    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 vToLight = light.m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    float fAttenuation = CalculateAttenuation(fDistance, light.m_fAttenuation, light.m_fAttRadius);
    if (fAttenuation <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    
    //float spotFactor = CalculateSpotEffect(lightDir, normalize(light.m_vDirection), light.m_fFalloff);
    float spotFactor = pow(max(dot(-vToLight, light.m_vDirection), 0.0f), light.m_fFalloff);
    
    if (spotFactor <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 viewDir = normalize(CameraPosition - vPosition);
    
    float diffuseFactor = CalculateDiffuse(vNormal, lightDir);
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor) +
                 (light.m_cSpecular.rgb * specularFactor * Material.SpecularColor);
                 
    return float4(lit * fAttenuation * spotFactor * light.m_fIntensity, 1.0);
#endif
}

float4 DirectionalLight(int nIndex, float3 vPosition, float3 vNormal)
{
#ifdef LIGHTING_MODEL_GOURAUD
#elif defined(LIGHTING_MODEL_LAMBERT)
#else
    LIGHT light = gLights[nIndex];
    if (light.m_bEnable == 0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(-light.m_vDirection);
    float3 viewDir = normalize(CameraPosition - vPosition);
    
    float diffuseFactor = CalculateDiffuse(vNormal, lightDir);
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.m_cDiffuse.rgb * diffuseFactor * Material.DiffuseColor) +
                 (light.m_cSpecular.rgb * specularFactor * Material.SpecularColor);
                 
    return float4(lit * light.m_fIntensity, 1.0);
#endif
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0, 0.0, 0.0, 0.0);
    float3 normalizedNormal = normalize(vNormal);
    
    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
#ifdef LIGHTING_MODEL_GOURAUD
            if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, vPosition, normalizedNormal);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, normalizedNormal);
            }
#elif defined(LIGHTING_MODEL_LAMBERT)
            if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, vPosition, normalizedNormal);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, normalizedNormal);
            }
#else
            if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, vPosition, normalizedNormal);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, normalizedNormal);
            }
            else if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                cColor += DirectionalLight(i, vPosition, normalizedNormal);
            }
#endif
        }
    }

    // Add global ambient light
    cColor += float4(gcGlobalAmbientLight.rgb * Material.AmbientColor, 0.0);
    cColor.a = 1.0;
    
    return cColor;
}

#ifdef LIGHTING_MODEL_LAMBERT
float4 ComputeLambertShading(float3 normal, float3 lightDir, float3 lightColor, float3 diffuseColor)
{
    float NdotL = max(dot(normal, lightDir), 0.0);
    return float4(lightColor * diffuseColor * NdotL, 1.0);
}
#endif

#ifdef LIGHTING_MODEL_BLINN_PHONG
float4 ComputeBlinnPhongShading(float3 normal, float3 lightDir, float3 viewDir, float3 lightColor,
                               float3 diffuseColor, float3 specularColor, float specularPower)
{
    float NdotL = max(dot(normal, lightDir), 0.0);
    float3 diffuse = lightColor * diffuseColor * NdotL;
    
    if (NdotL > 0.0) 
    {
        float3 halfVector = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfVector), 0.0);
        float3 specular = lightColor * specularColor * pow(NdotH, max(specularPower, 1.0) * 4.0);
        return float4(diffuse + specular, 1.0);
    }
    
    return float4(diffuse, 1.0);
}
#endif
