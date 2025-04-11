
SamplerState Sampler : register(s0);

////////
/// 공용: 13 ~ 15
///////
cbuffer ObjectBuffer : register(b13)
{
    row_major matrix WorldMatrix;
    row_major matrix InverseTransposedWorld;
    float4 UUID;
    bool IsSelected;
    float3 ObjectPadding;
};

cbuffer ViewBuffer : register(b14)
{
    row_major matrix ViewMatrix;
    row_major matrix InvViewMatrix;
    float3 ViewLocation;
    float ViewPadding;
}

cbuffer ProjectionBuffer : register(b15)
{
    row_major matrix ProjectionMatrix;
    row_major matrix InvProjectionMatrix;
    float NearClip;
    float FarClip;
    float2 ProjectionPadding;
}
