// MatrixBuffer: 변환 행렬 관리
cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float pad;
};

struct VS_INPUT
{
    float4 position : POSITION; // 버텍스 위치
    float4 color : COLOR; // 버텍스 색상
    float3 normal : NORMAL; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간으로 변환된 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치 (조명용)
    float4 color : COLOR; // 버텍스 컬러 또는 머티리얼 베이스 컬러
    float3 normal : NORMAL; // 월드 공간 노멀
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그 (1.0 또는 0.0)
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.materialIndex = input.materialIndex;
    
    float4 worldPosition = mul(input.position, Model);
    
    output.worldPos = worldPosition.xyz;
    
    float4 viewPosition = mul(worldPosition, View);
    
    output.position = mul(viewPosition, Projection);
    
    output.color = input.color;
  
    // 입력 normal 값의 길이 확인
    float normalThreshold = 0.001;
    float normalLen = length(input.normal);
    
    if (normalLen < normalThreshold)
    {
        output.normalFlag = 0.0;
    }
    else
    {
        output.normal = normalize(mul(input.normal, (float3x3) MInverseTranspose));
        output.normalFlag = 1.0;
    }
    output.texcoord = input.texcoord;
    
    return output;
}