cbuffer TileLightCullSettings : register(b0)
{
    uint2 ScreenSize; // 화면 크기
    uint Enable25DCulling; // 2.5D 컬링 ON/OFF
    float Padding; // 16바이트 정렬 맞춤
}

[numthreads(1, 1, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID)
{
}
