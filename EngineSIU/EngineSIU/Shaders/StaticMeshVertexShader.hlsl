
#include "ShaderRegisters.hlsl"

#ifdef LIGHTING_MODEL_GOURAUD
#include "Light.hlsl"
#endif

PS_INPUT_StaticMesh mainVS(VS_INPUT_StaticMesh Input)
{
    PS_INPUT_StaticMesh Output;

    Output.Position = float4(Input.Position, 1.0);
    Output.Position = mul(Output.Position, WorldMatrix);
    Output.WorldPosition = Output.Position.xyz;
    
    Output.Position = mul(Output.Position, ViewMatrix);
    Output.Position = mul(Output.Position, ProjectionMatrix);
    
    Output.WorldNormal = mul(Input.Normal, (float3x3)InverseTransposedWorld);
    
    Output.UV = Input.UV;
    Output.Color = Input.Color;
    Output.MaterialIndex = Input.MaterialIndex;

#ifdef LIGHTING_MODEL_GOURAUD
    float4 litColor = Lighting(worldPosition.xyz, worldNormal);
    output.color = float4(litColor.rgb, 1.0);
#else
    output.color = input.color;
#endif
    
    return Output;
}
