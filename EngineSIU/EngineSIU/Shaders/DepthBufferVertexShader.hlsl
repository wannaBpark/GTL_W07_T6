// 입력 구조체: POSITION과 TEXCOORD
struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

// 출력 구조체
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

VS_OUTPUT mainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = float4(input.position.xy, 1.0, 1.0);
    output.texCoord = input.texCoord;
    return output;
}
