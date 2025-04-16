
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3

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

cbuffer Lighting : register(b0)
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

float4 PointLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float WorldViewPosition)
{
#ifdef LIGHTING_MODEL_GOURAUD
    FPointLightInfo light = PointLights[nIndex];
    
    float3 vToLight = light.Position - WorldPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.Radius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor=CalculateDiffuse(WorldNormal, lightDir);
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    
    float specularFactor = 0.0;
    if (diffuseFactor > 0.0)
    {
        float3 viewDir = normalize(WorldViewPosition - WorldPosition);
        float3 halfVector = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(WorldNormal, halfVector), 0.0), 4.0);
    }
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) + (specularFactor * Material.SpecularColor);
                 
    return float4(lit * attenuation * light.Intensity, 1.0);
    
#elif defined(LIGHTING_MODEL_LAMBERT)

    FPointLightInfo light = PointLights[nIndex];
    
    float3 vToLight = light.Position - WorldPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.Radius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor=CalculateDiffuse(WorldNormal,lightDir);
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor);
    // bHasDiffuseMap
    if (Material.TextureFlag & 1 << 1)
    {
        lit = (light.LightColor.rgb * diffuseFactor);
    }
    return float4(lit * attenuation * light.Intensity, 1.0);
    
#else
    FPointLightInfo light = PointLights[nIndex];
    
    float3 vToLight = light.Position - WorldPosition;
    float fDistance = length(vToLight);
    
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    if (attenuation <= 0.0)
        return float4(0.f, 0.f, 0.f, 0.f);
    
    float3 lightDir = normalize(vToLight);
    
    float3 viewDir = normalize(WorldViewPosition - WorldPosition);
    
    float diffuseFactor = CalculateDiffuse(WorldNormal, lightDir);
    float specularFactor = CalculateSpecular(WorldNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) + (specularFactor * Material.SpecularColor);
    // bHasDiffuseMap
    if (Material.TextureFlag & 1 << 1)
    {
        lit = (light.LightColor.rgb * diffuseFactor) + (specularFactor * Material.SpecularColor);
    }
    
    return float4(lit * attenuation * light.Intensity, 1.0);
#endif
}

// 기존 방식과 달라서 att부분 수정이 필요함.
float4 SpotLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition)
{
#ifdef LIGHTING_MODEL_GOURAUD
    FSpotLightInfo light = SpotLights[nIndex];
    
    float3 vToLight = light.Position - WorldPosition;
    float fDistance = length(vToLight);
    
    if (fDistance > light.Radius)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor =CalculateDiffuse(WorldNormal, lightDir);
    float attenuationDistance = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    float spotFactor = CalculateSpotEffect(lightDir, normalize(light.Direction), light.InnerRad, light.OuterRad, light.Attenuation);
    
    float specularFactor = 0.0;
    if (diffuseFactor > 0.0)
    {
        float3 viewDir = normalize(WorldViewPosition - WorldPosition);
        float3 halfVector = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(WorldNormal, halfVector), 0.0), 4.0);
    }
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) + (specularFactor * Material.SpecularColor);
    
    return float4(lit * attenuationDistance * spotFactor * light.Intensity, 1.0);
#elif defined(LIGHTING_MODEL_LAMBERT)
    FSpotLightInfo light = SpotLights[nIndex];
    
    float3 vToLight = light.Position - WorldPosition;
    float fDistance = length(vToLight);
    
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    if (attenuation <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float diffuseFactor = CalculateDiffuse(WorldNormal, lightDir);
    float spotFactor = CalculateSpotEffect(lightDir, normalize(light.Direction), light.InnerRad, light.OuterRad, light.Attenuation);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor);
    // bHasDiffuseMap
    if (Material.TextureFlag & 1 << 1)
    {
        lit = (light.LightColor.rgb * diffuseFactor);
    }
    
    return float4(lit * attenuation* spotFactor * light.Intensity, 1.0);
    // End test
#else
    FSpotLightInfo light = SpotLights[nIndex];

    float3 vToLight = light.Position - WorldPosition;
    float fDistance = length(vToLight);
    
    float attenuation = CalculateAttenuation(fDistance, light.Attenuation, light.Radius);
    if (attenuation <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightDir = normalize(vToLight);
    float spotFactor = CalculateSpotEffect(lightDir, normalize(light.Direction), light.InnerRad, light.OuterRad, 1);
    
    if (spotFactor <= 0.0)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 viewDir = normalize(WorldViewPosition - WorldPosition);
    
    float diffuseFactor = CalculateDiffuse(WorldNormal, lightDir);
    float specularFactor = CalculateSpecular(WorldNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) + (specularFactor * Material.SpecularColor);
    // bHasDiffuseMap
    if (Material.TextureFlag & 1 << 1)
    {
        lit = (light.LightColor.rgb * diffuseFactor) + (specularFactor * Material.SpecularColor);
    }
    
    return float4(lit * attenuation * spotFactor * light.Intensity, 1.0);    
#endif
}

float4 DirectionalLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition)
{
    FDirectionalLightInfo light = Directional[nIndex];
    
    float3 lightDir = normalize(-light.Direction);
    float3 viewDir = normalize(WorldViewPosition - WorldPosition);
    float diffuseFactor = CalculateDiffuse(WorldNormal, lightDir);
    
#ifdef LIGHTING_MODEL_GOURAUD
    float specularFactor = CalculateSpecular(WorldNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) +
                 (specularFactor * Material.SpecularColor);
                 
#elif defined(LIGHTING_MODEL_LAMBERT)
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor);
    
    // bHasDiffuseMap
    if (Material.TextureFlag & 1 << 1)
    {
        lit = light.LightColor.rgb * diffuseFactor;
    }
#else
    float specularFactor = CalculateSpecular(WorldNormal, lightDir, viewDir, Material.SpecularScalar);
    
    float3 lit = (light.LightColor.rgb * diffuseFactor * Material.DiffuseColor) + (specularFactor * Material.SpecularColor);
    // bHasDiffuseMap
    if (Material.TextureFlag & 1 << 1)
    {
        lit = (light.LightColor.rgb * diffuseFactor) + (specularFactor * Material.SpecularColor);
    }

#endif
    return float4(lit * light.Intensity, 1.0);
}

float4 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition)
{
    float4 FinalColor = float4(0.0, 0.0, 0.0, 0.0);
    
    // 다소 비효율적일 수도 있음.
    [unroll(MAX_POINT_LIGHT)]
    for (int i = 0; i < PointLightsCount; i++)
    {
        FinalColor += PointLight(i, WorldPosition, WorldNormal, WorldViewPosition);
    }    
    [unroll(MAX_SPOT_LIGHT)]
    for (int j = 0; j < SpotLightsCount; j++)
    {
        FinalColor += SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition);
    }
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        FinalColor += DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition);
    }

    // Add global ambient light
    FinalColor += float4(Ambient.AmbientColor.rgb, 0.0);
    FinalColor.a = 1.0;
    
    return FinalColor;
}
