
struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    
    float3 SpecularColor;
    float SpecularScalar;
    
    float3 EmissiveColor;
    float DensityScalar;
    
    float3 AmbientColor;
    float MaterialPad0;
};

struct VS_INPUT_StaticMesh
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
    int MaterialIndex : MATERIAL_INDEX;
};

struct PS_INPUT_StaticMesh
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
    float3 WorldPosition : TEXCOORD1;
    int MaterialIndex : MATERIAL_INDEX;
};

////////
/// 공용: 14 ~ 15
///////
cbuffer ObjectBuffer : register(b14)
{
    row_major matrix WorldMatrix;
    row_major matrix InverseTransposedWorld;
    
    float4 UUID;
    
    bool bIsSelected;
    float3 ObjectPadding;
};

/**
 * 기존에는 View 버퍼와 Projection 버퍼의
 * 업데이트 시기가 달라서 분리했지만, 
 * 여러 뷰포트를 렌더하는 경우 다음 뷰포트로 넘어가면
 * Projection 버퍼를 업데이트해야 하므로,
 * 버퍼를 분리하는 의미가 사라졌으므로 통합.
 */
cbuffer CameraBuffer : register(b15)
{
    row_major matrix ViewMatrix;
    row_major matrix InvViewMatrix;
    
    row_major matrix ProjectionMatrix;
    row_major matrix InvProjectionMatrix;
    
    float3 ViewWorldLocation;
    float ViewPadding;
    
    float NearClip;
    float FarClip;
    float2 ProjectionPadding;
}
