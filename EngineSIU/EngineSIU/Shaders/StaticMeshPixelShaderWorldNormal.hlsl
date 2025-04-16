
#include "ShaderRegisters.hlsl"

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(Input.WorldNormal, 1.f);
    
    FinalColor = (FinalColor + 1.f) / 2.f;
    
    return FinalColor;
}
