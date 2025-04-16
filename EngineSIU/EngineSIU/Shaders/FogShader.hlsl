Texture2D DepthTexture : register(t103);
SamplerState Sampler : register(s0);

cbuffer FogConstant : register(b0)
{
    row_major float4x4 InvView;
    row_major float4x4 InvProj;

    float4 FogColor;
    
    float StartDistance;
    float EndDistance;    
    float FogHeight;
    float FogHeightFalloff;
    
    float FogDensity;
    float FogDistanceWeight;
    float2 padding;
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

float3 ReconstructWorldPos(float2 UV, float Depth)
{
    float4 NDC;
    NDC.xy = UV * 2.0 - 1.0;  // [0,1] → [-1,1]
    NDC.y *= -1;
    NDC.z = Depth;
    NDC.w = 1.0;

    float4 WorldPos = mul(NDC, InvProj);
    WorldPos /= WorldPos.w;

    WorldPos = mul(WorldPos, InvView);

    return WorldPos.xyz;
}

float GetCameraDistance(float Depth)
{
    float Z = Depth;
    float4 ClipPos = float4(0, 0, Z, 1);
    float4 ViewPos = mul(ClipPos, InvProj);
    float ViewZ = ViewPos.z / ViewPos.w;
    return abs(ViewZ); // 카메라 기준 거리
}

float ComputeFogFactor(float3 WorldPos, float Depth)
{
    float3 CameraPos = InvView[3].xyz;

    float CameraDist = GetCameraDistance(Depth);
    float DistFactor = saturate((CameraDist - StartDistance) / (EndDistance - StartDistance));

    // 높이 기반 안개
    float HeightDiff = max(0.0f, FogHeight - WorldPos.z);
    float HeightFactor = 1.0 - exp(-HeightDiff * FogHeightFalloff);
    float HeightFogFactor = HeightFactor * FogDensity * DistFactor; // <- 거리 영향 추가됨

    // TODO: 변경하면서 잘 작동하는 지 모르겠음.
    // 거리 기반 안개 (카메라가 안개 안에 있을 때만)
    float CameraHeightDiff = max(0.0f, FogHeight - CameraPos.z);
    float CameraHeightFactor = 1.0 - exp(-CameraHeightDiff * FogHeightFalloff);

    float DistFogFactor = DistFactor * FogDensity * FogDistanceWeight * (CameraHeightFactor);


    float FogFactor = HeightFogFactor + DistFogFactor;
    // 안전한 클램핑
    FogFactor = saturate(FogFactor);

    return FogFactor;
}

PS_INPUT mainVS(uint VertexID : SV_VertexID)
{
    PS_INPUT Output;

    float2 QuadPositions[6] = {
        float2(-1,  1),  // Top Left
        float2(1,  1),  // Top Right
        float2(-1, -1),  // Bottom Left
        float2(1,  1),  // Top Right
        float2(1, -1),  // Bottom Right
        float2(-1, -1)   // Bottom Left
    };

    float2 UVs[6] = {
        float2(0, 0), float2(1, 0), float2(0, 1),
        float2(1, 0), float2(1, 1), float2(0, 1)
    };

    Output.Position = float4(QuadPositions[VertexID], 0, 1);
    Output.UV = UVs[VertexID];

    return Output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float2 UV = input.UV;
    float rawDepth = DepthTexture.Sample(Sampler, UV).r;
    float3 WorldPos = ReconstructWorldPos(UV, rawDepth);
    
    float FogFactor = ComputeFogFactor(WorldPos, rawDepth);

    float4 FinalColor = float4(FogColor.rgb, FogFactor);
    return FinalColor;
}
