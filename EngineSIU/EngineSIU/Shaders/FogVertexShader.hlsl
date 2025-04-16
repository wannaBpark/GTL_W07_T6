
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

VS_OUTPUT mainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.Position = float4(input.Position.xy, 1.0f, 1.0f);
    output.UV = input.UV;
    
    return output;
}
