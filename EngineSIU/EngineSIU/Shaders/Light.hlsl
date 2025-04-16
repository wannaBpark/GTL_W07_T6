#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3

#define MAX_LIGHT_PER_TILE 256

//struct LIGHT
//{
//    float3 m_cDiffuse; // 광원의 확산 색상
//    float Pad0;    

//    float3 m_vPosition; // 광원의 위치 (Point, Spot)
//    float m_fFalloff; // 스팟라이트의 감쇠 인자

//    float3 m_vDirection; // 광원의 방향 (Spot, Directional)
//    float Pad2;

//    float m_fAttenuation; // 거리 기반 감쇠 계수
//    int m_bEnable; // 광원 활성화 여부
//    int m_nType; // 광원 유형
//    float m_fIntensity; // 광원 강도
    
//    float m_fAttRadius; // 감쇠 반경 (Attenuation Radius)
//    float3 Pad3;
    
//    float m_fInnerCos; // cos(inner angle)
//    float m_fOuterCos; // cos(outer angle)
//    float2 Pad4; // 정렬용
//};

//cbuffer cbLights : register(b2)
//{
//    LIGHT gLights[MAX_LIGHTS];
//    float4 gcGlobalAmbientLight;
//    int gnLights;
//    float3 Pad0;
//};


struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
};

struct FPointLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    int Type;
    float Intensity;
    float Attenuation;
    float Padding;
};

struct FSpotLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    float3 Direction;
    float Intensity;

    int Type;
    float InnerRad;
    float OuterRad;
    float Attenuation;
};


cbuffer Lighting : register(b2)
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

cbuffer TileLightCullSettings : register(b8)
{
    uint2 ScreenSize; // 화면 해상도
    uint2 TileSize; // 한 타일의 크기 (예: 16x16)

    float NearZ; // 카메라 near plane
    float FarZ; // 카메라 far plane

    row_major matrix ViewMatrix; // View 행렬
    row_major matrix ProjectionMatrix; // Projection 행렬
    row_major matrix InverseProjection; // Projection^-1, 뷰스페이스 복원용

    uint NumLights; // 총 라이트 수
    uint Enable25DCulling; // 1이면 2.5D 컬링 사용
}

struct LightPerTiles
{
    uint NumLights;
    uint Indices[MAX_LIGHT_PER_TILE];
    uint Padding[3];
};
StructuredBuffer<FPointLightInfo> gPointLights : register(t10);
StructuredBuffer<LightPerTiles> gLightPerTiles : register(t20);

float CalculateAttenuation(float distance, float attenuationFactor, float radius)
{
    if (distance > radius)
        return 0.0;
        
    return 1.0 / (1.0 + attenuationFactor * distance * distance);
}

//float CalculateSpotEffect(float3 lightDir, float3 spotDir, float spotFalloff)
//{
//    float cosAngle = dot(-lightDir, spotDir);
//    return pow(max(cosAngle, 0.0), spotFalloff);
//}

float CalculateSpotEffect(float3 lightDir, float3 spotDir, float innerRad, float outerRad, float spotFalloff)
{
    float dotProduct = dot(-lightDir, spotDir); //[-1,1]이고
    
    float spotEffect = smoothstep(cos(outerRad/2), cos(innerRad/2), dotProduct);
    
    return spotEffect * pow(max(dotProduct, 0.0), 1);
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
    return pow(max(dot(normal, halfVector), 0.0), max(specularPower, 1.0) * 5.0);
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal)
{
#ifdef LIGHTING_MODEL_GOURAUD
    FPointLightInfo light = PointLights[nIndex];
    
    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.Radius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor=CalculateDiffuse(vNormal, lightDir);
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    
    float specularFactor = 0.0;
    if (diffuseFactor > 0.0)
    {
// Begin Test
        float3 viewDir = normalize(CameraPosition - vPosition);
        float3 halfVector = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(vNormal, halfVector), 0.0), 4.0);
// End Test
    }
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuation * light.Intensity, 1.0);
    
#elif defined(LIGHTING_MODEL_LAMBERT)

    FPointLightInfo light = PointLights[nIndex];
    
    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.Radius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor=CalculateDiffuse(vNormal,lightDir);
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    
    float3 lit = light.LightColor.rgb * diffuseFactor * Material.DiffuseColor;
                 
    return float4(lit * attenuation * light.Intensity, 1.0);
    
#else
    FPointLightInfo light = PointLights[nIndex];
    
    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);
    
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    if (attenuation <= 0.0)
        return float4(0.f, 0.f, 0.f, 0.f);
    
    float3 lightDir = normalize(vToLight);
    
    float3 viewDir = normalize(CameraPosition - vPosition);
    
    float diffuseFactor = CalculateDiffuse(vNormal, lightDir);
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuation * light.Intensity, 1.0);
#endif
}

// 기존 방식과 달라서 att부분 수정이 필요함.
float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
#ifdef LIGHTING_MODEL_GOURAUD
    FSpotLightInfo light = SpotLights[nIndex];
    
    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.Radius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor =CalculateDiffuse(vNormal, lightDir);
    float attenuationDistance = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    float spotFactor = CalculateSpotEffect(lightDir, normalize(light.Direction), light.InnerRad, light.OuterRad, light.Attenuation);
    
    float specularFactor = 0.0;
    if (diffuseFactor > 0.0)
    {
        float3 viewDir = normalize(CameraPosition - vPosition);
        float3 halfVector = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(vNormal, halfVector), 0.0), 4.0);
    }
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuationDistance * spotFactor * light.Intensity, 1.0);
#elif defined(LIGHTING_MODEL_LAMBERT)
    FSpotLightInfo light = SpotLights[nIndex];
    
    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);
    
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    if (attenuation <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor =CalculateDiffuse(vNormal, lightDir);
    float spotFactor = CalculateSpotEffect(lightDir, normalize(light.Direction), light.InnerRad, light.OuterRad, light.Attenuation);
    
    // Lambert에서는 specular 계산 없음
    float3 lit = light.LightColor.rgb * diffuseFactor * Material.DiffuseColor;
                 
    return float4(lit * attenuation* spotFactor * light.Intensity, 1.0);
    // End test
#else
    FSpotLightInfo light = SpotLights[nIndex];

    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);
    
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    if (attenuation <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float spotFactor = CalculateSpotEffect(lightDir, normalize(light.Direction), light.InnerRad, light.OuterRad, 1);
    
    if (spotFactor <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 viewDir = normalize(CameraPosition - vPosition);
    
    float diffuseFactor = CalculateDiffuse(vNormal, lightDir);
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuation * spotFactor * light.Intensity, 1.0);    
#endif
}

float4 DirectionalLight(int nIndex, float3 vPosition, float3 vNormal)
{
    FDirectionalLightInfo light = Directional[nIndex];
    
    float3 lightDir = normalize(-light.Direction);
    float3 viewDir = normalize(CameraPosition - vPosition);
    float diffuseFactor = CalculateDiffuse(vNormal, lightDir);
    
#ifdef LIGHTING_MODEL_GOURAUD
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
                 
#elif defined(LIGHTING_MODEL_LAMBERT)
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor);
                 
#else
    float specularFactor = CalculateSpecular(vNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
#endif
    return float4(lit * light.Intensity, 1.0);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0, 0.0, 0.0, 0.0);
    float3 normalizedNormal = normalize(vNormal);
    
    // 다소 비효율적일 수도 있음.
    [unroll(MAX_POINT_LIGHT)]
    for (int i = 0; i < PointLightsCount; i++)
    {
        cColor += PointLight(i, vPosition, normalizedNormal);
    }    
    [unroll(MAX_SPOT_LIGHT)]
    for (int j = 0; j < SpotLightsCount; j++)
    {
        cColor += SpotLight(j, vPosition, normalizedNormal);
    }
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        cColor += DirectionalLight(k, vPosition, normalizedNormal);
    }

    // Add global ambient light
    cColor += float4(Ambient.AmbientColor.rgb, 0.0);
    cColor.a = 1.0;
    
    return cColor;
}
