Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}

cbuffer UUIDConstant : register(b2)
{
    float4 UUID;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 uuid : SV_Target1;
};

float4 main(PSInput input) : SV_TARGET
{
    PSOutput output;
    
    float2 uv = input.texCoord + float2(indexU, indexV);
    float4 col = gTexture.Sample(gSampler, uv);
    float threshold = 0.1f;
    
    // 검정색에 가까운 경우 알파 값을 0으로 설정하여 투명하게 처리
    if (col.r < threshold && col.g < threshold && col.b < threshold)
    {
        discard;
    }
    else
    {
        output.color = col;
    }
    
    output.uuid = UUID;
    
    return output.color;
}