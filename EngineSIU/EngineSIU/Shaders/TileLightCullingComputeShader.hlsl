// 상수 정의
#define TILE_SIZE 32
#define MAX_LIGHTS_PER_TILE 1024
#define SHADER_ENTITY_TILE_BUCKET_COUNT (MAX_LIGHTS_PER_TILE / 32)
#define THREAD_GROUP_SIZE 8
#define NUM_SLICES 32    // 타일 내 depth를 32개의 슬라이스로 분할 (시그래프 Harada 값과 동일)

cbuffer TileLightCullSettings : register(b0)
{
    uint2 ScreenSize; // 화면 해상도
    uint2 TileSize; // 한 타일의 크기 (예: 16x16)

    float NearZ; // 카메라 near plane
    float FarZ; // 카메라 far plane

    row_major matrix View; // View 행렬
    row_major matrix Projection; // Projection 행렬
    row_major matrix InverseProjection; // Projection^-1, 뷰스페이스 복원용

    uint NumLights; // 총 라이트 수
    uint Enable25DCulling; // 1이면 2.5D 컬링 사용
}
struct Sphere
{
    float3 c; // Center point.
    float r; // Radius.
};
struct Plane
{
    float3 N; // Plane normal.
    float d; // Distance to origin.
};
struct Frustum
{
    Plane planes[4]; // left, right, top, bottom frustum planes.
};

Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;

    float3 v0 = p1 - p0;
    float3 v2 = p2 - p0;

    plane.N = normalize(cross(v0, v2));

	// Compute the distance to the origin using p0.
    plane.d = dot(plane.N, p0);

    return plane;
}

// Convert clip space coordinates to view space
float4 ClipToView(float4 clip)
{
	// View space position.
    float4 view = mul(clip, InverseProjection);
	// Perspective projection.
    view = view / view.w;

    return view;
}
// Convert screen space coordinates to view space.
float4 ScreenToView(float4 screen, float2 dim_rcp)
{
	// Convert to normalized texture coordinates // Convert to clip space
    float2 texCoord = screen.xy * dim_rcp;
    float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);
    return ClipToView(clip);
}

bool SphereInsidePlane(Sphere sphere, Plane plane)
{
    return dot(plane.N, sphere.c) - plane.d < -sphere.r;
}
bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear, float zFar) // this can only be used in view space
{
    bool result = true;
    result = ((sphere.c.z + sphere.r < zNear || sphere.c.z - sphere.r > zFar) ? false : result);
    result = ((SphereInsidePlane(sphere, frustum.planes[0])) ? false : result);
    result = ((SphereInsidePlane(sphere, frustum.planes[1])) ? false : result);
    result = ((SphereInsidePlane(sphere, frustum.planes[2])) ? false : result);
    result = ((SphereInsidePlane(sphere, frustum.planes[3])) ? false : result);

    return result;
}

inline uint2 unflatten2D(uint idx, uint2 dim)
{
    return uint2(idx % dim.x, idx / dim.x);
}

struct FLightGPU
{
    float3 Position;
    float Radius;
    float3 Direction; // 사용하지 않음
    uint isPointLight;
};

StructuredBuffer<FLightGPU> LightBuffer : register(t0); // CPU에서 전달된 PointLight들
Texture2D<float> gDepthTexture : register(t1);          // Depth Texture

RWStructuredBuffer<uint> TileLightMask : register(u0); // 타일별 조명 마스크
RWTexture2D<float4> DebugHeatmap : register(u3); // 디버깅용 히트맵


// Group Shared 메모리 - Depth Masking 누적에 쓰입니다~
groupshared uint tileDepthMask;

// dispatchID = groupID * [numthreads] + threadID

// no difference between [1,1,1] - 그러나 DepthMap Texturing할 때 한 픽셀의 값 읽어오려면 TILE_SIZE만큼 나눠야 효율적
[numthreads(TILE_SIZE, TILE_SIZE, 1)] 
void mainCS(uint3 groupID : SV_GroupID, uint3 dispatchID : SV_DispatchThreadID, uint3 threadID : SV_GroupThreadID)
{
    uint2 tileCoord = groupID.xy;
    uint2 pixel = tileCoord * TILE_SIZE + threadID.xy;
    
    uint2 screenTileSize = TileSize;  // TILE_SIZE
    uint2 screenSize = ScreenSize;
    
    // --- 1. 타일 내 각 픽셀의 Depth를 샘플링하고, 해당 슬라이스 인덱스의 비트를 그룹 공유 변수에 누적
    // 초기화: 그룹의 첫 번째 스레드가 tileDepthMask를 0으로 초기화
    if (threadID.x == 0 && threadID.y == 0)
    {
        tileDepthMask = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
     // 픽셀이 화면 내에 속하는지 검사 후,
    float depthSample = 0;
    if (all(pixel < screenSize))
    {
        depthSample = gDepthTexture[pixel];
    }
    // 선형 깊이로 변환:
    // depthSample이 1.0이면 FarZ로 처리하여 수치 불안정을 방지 (흠)
    float linearZ = (depthSample == 1.0f)
    ? FarZ
    : (NearZ * FarZ) / (FarZ - depthSample * (FarZ - NearZ));
    float depthNormalized = saturate((linearZ - NearZ) / (FarZ - NearZ));
    
    // 슬라이스 인덱스 계산: NUM_SLICES개의 슬라이스로 나누기
    int sliceIndex = (int) floor(depthNormalized * NUM_SLICES);
    sliceIndex = clamp(sliceIndex, 0, NUM_SLICES - 1);
    
    // 각 픽셀의 스레드가 해당 슬라이스 해당하는 비트를 1로 설정
    // Comment) 즉 스레드 하나가 픽셀 하나의 depth마스킹 수행
    uint sliceBit = 1u << sliceIndex;
    InterlockedOr(tileDepthMask, sliceBit);     // Atomic OR (그룹 공유에 누적)
    
    GroupMemoryBarrierWithGroupSync();

    // 뷰 공간 프러스텀 계산
    float2 dim_rcp = 1.0 / float2(screenSize);
    float tilePixelWidth = screenTileSize.x;
    float tilePixelHeight = screenTileSize.y;

    float2 tileMin = tileCoord * screenTileSize; // GroupID 기준 tile Min, Max
    float2 tileMax = tileMin + screenTileSize;

    // 8개 코너의 clip space 점을 inverse projection하여 view space로 변환
    float3 corners[8];
    float nearZ = NearZ;
    float farZ = FarZ;

    // tile clip corner 좌표 → view 변환
    float3 viewCorners[8];
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        float2 uv = float2((i & 1) ? tileMax.x : tileMin.x, (i & 2) ? tileMax.y : tileMin.y);
        // i=0: (tileMin.x, tileMin.y) → 왼쪽 상단
        // i=1: (tileMax.x, tileMin.y) → 오른쪽 상단
        // i=2: (tileMin.x, tileMax.y) → 왼쪽 하단
        // i=3: (tileMax.x, tileMax.y) → 오른쪽 하단
        uv /= screenSize;
        uv.y = 1.0 - uv.y; // y축 반전해야 카메라 위로 들어올렸을 때 사각형은 아래로 감
        float4 clipNear = float4(uv * 2.0 - 1.0, nearZ, 1.0);
        float4 clipFar =  float4(uv * 2.0 - 1.0, farZ , 1.0);

        float4 viewNear = mul(clipNear, InverseProjection);
        float4 viewFar =  mul(clipFar , InverseProjection);
        viewCorners[i + 0] = viewNear.xyz / viewNear.w;
        viewCorners[i + 4] = viewFar.xyz / viewFar.w;
    }

    // 프러스텀 4개 면 생성
    Frustum frustum;
    frustum.planes[0] = ComputePlane(viewCorners[0], viewCorners[2], viewCorners[6]); // left
    frustum.planes[1] = ComputePlane(viewCorners[3], viewCorners[1], viewCorners[7]); // right
    frustum.planes[2] = ComputePlane(viewCorners[1], viewCorners[0], viewCorners[5]); // top
    frustum.planes[3] = ComputePlane(viewCorners[2], viewCorners[3], viewCorners[6]); // bottom

    // 타일 인덱스
    uint flatTileIndex = tileCoord.y * (ScreenSize.x / screenTileSize.x) + tileCoord.x;

    // 마스킹 초기화
    uint mask = 0;
    uint hitCount = 0;

    [loop]
    for (uint i = 0; i < NumLights; ++i)
    {
        FLightGPU light = LightBuffer[i];

        Sphere s;
        s.c = mul(float4(light.Position, 1), View).xyz;
        s.r = light.Radius;
        
        if (SphereInsideFrustum(s, frustum, nearZ, farZ))
        {
            uint bucketIdx = i / 32;
            uint bitIdx = i % 32;
            
            InterlockedOr(TileLightMask[flatTileIndex * SHADER_ENTITY_TILE_BUCKET_COUNT + bucketIdx], 1 << bitIdx);
            hitCount++; 
        }
    }
    
    // heatmap 시각화
    const float3 heatmap[] = {
        float3(0, 0, 0),
        float3(0, 0, 1),
        float3(0, 1, 1),
        float3(0, 1, 0),
        float3(1, 1, 0),
        float3(1, 0, 0),
    };
    const float maxHeat = 50.0f;
    float l = saturate(hitCount / maxHeat) * 5;
    float3 c1 = heatmap[floor(l)];
    float3 c2 = heatmap[ceil(l)];
    float3 color = lerp(c1, c2, frac(l));
    float4 result = float4(color, 0.8f);

    // 타일 내부 모든 픽셀에 출력
    for (uint i = 0; i < TILE_SIZE * TILE_SIZE; ++i)
    {
        uint2 local = unflatten2D(i, uint2(TILE_SIZE, TILE_SIZE));
        // pixel = 타일 내부에서 0부터 TILE_SIZE−1까지의 오프셋
        // tileCoord * TILE_SIZE = 타일의 왼쪽 상단 좌표
        uint2 pixel = tileCoord * TILE_SIZE + local;
        if (all(pixel < screenSize))
        {
            DebugHeatmap[pixel] = result;
        }
    }
    //for (uint i = 0; i < screenSize.x * screenSize.y; ++i)
    //{
    //    uint2 pixel = unflatten2D(i, screenSize);
    //    DebugHeatmap[pixel] = float4(heatmap[3].xyz, 1.0f);
    //}

    // SV_DispatchThreadID는 전체 화면상의 픽셀 좌표(전역 좌표)를 나타냅니다.
    
}
