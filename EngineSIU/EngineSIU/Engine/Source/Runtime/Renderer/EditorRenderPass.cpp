#include "EditorRenderPass.h"

FEditorRenderPass::FEditorRenderPass()
{
}

FEditorRenderPass::~FEditorRenderPass()
{
}

void FEditorRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
}

void FEditorRenderPass::PrepareRender()
{
}

void FEditorRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FEditorRenderPass::ClearRenderArr()
{
}

void FEditorRenderPass::CreateBuffers()
{
    ////////////////////////////////////
    // Box 버퍼 생성
    TArray<FVector> CubeFrameVertices;
    CubeFrameVertices.Add({ -1.f, -1.f, -1.f }); // 0
    CubeFrameVertices.Add({ -1.f, 1.f, -1.f }); // 1
    CubeFrameVertices.Add({ 1.f, -1.f, -1.f }); // 2
    CubeFrameVertices.Add({ 1.f, 1.f, -1.f }); // 3
    CubeFrameVertices.Add({ -1.f, -1.f, 1.f }); // 4
    CubeFrameVertices.Add({ 1.f, -1.f, 1.f }); // 5
    CubeFrameVertices.Add({ -1.f, 1.f, 1.f }); // 6
    CubeFrameVertices.Add({ 1.f, 1.f, 1.f }); // 7

    TArray<uint32> CubeFrameIndices = {
        // Bottom face
        0, 1, 1, 3, 3, 2, 2, 0,
        // Top face
        4, 6, 6, 7, 7, 5, 5, 4,
        // Side faces
        0, 4, 1, 6, 2, 5, 3, 7
    };

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(FVector) * CubeFrameVertices.Num();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = CubeFrameVertices.GetData();

    HRESULT hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(uint32) * CubeFrameIndices.Num();

    initData.pSysMem = CubeFrameIndices.GetData();

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Index);

    Resources.Primitives.Box.NumVertices = CubeFrameVertices.Num();
    Resources.Primitives.Box.VertexStride = sizeof(FVector);
    Resources.Primitives.Box.NumIndices = CubeFrameIndices.Num();

    ////////////////////////////////////
    // Sphere 버퍼 생성
    FVector SphereFrameVertices[] =
    {
        {1.0, 0.0, 0},
        {0.9795299412524945, 0.20129852008866006, 0},
        {0.9189578116202306, 0.39435585511331855, 0},
        {0.8207634412072763, 0.5712682150947923, 0},
        {0.6889669190756866, 0.72479278722912, 0},
        {0.5289640103269624, 0.8486442574947509, 0},
        {0.3473052528448203, 0.9377521321470804, 0},
        {0.1514277775045767, 0.9884683243281114, 0},
        {-0.05064916883871264, 0.9987165071710528, 0},
        {-0.2506525322587204, 0.9680771188662043, 0},
        {-0.4403941515576344, 0.8978045395707416, 0},
        {-0.6121059825476629, 0.7907757369376985, 0},
        {-0.758758122692791, 0.6513724827222223, 0},
        {-0.8743466161445821, 0.48530196253108104, 0},
        {-0.9541392564000488, 0.29936312297335804, 0},
        {-0.9948693233918952, 0.10116832198743228, 0},
        {-0.9948693233918952, -0.10116832198743204, 0},
        {-0.9541392564000489, -0.29936312297335776, 0},
        {-0.8743466161445822, -0.4853019625310808, 0},
        {-0.7587581226927911, -0.651372482722222, 0},
        {-0.6121059825476627, -0.7907757369376986, 0},
        {-0.44039415155763423, -0.8978045395707417, 0},
        {-0.2506525322587205, -0.9680771188662043, 0},
        {-0.05064916883871266, -0.9987165071710528, 0},
        {0.15142777750457667, -0.9884683243281114, 0},
        {0.3473052528448203, -0.9377521321470804, 0},
        {0.5289640103269624, -0.8486442574947509, 0},
        {0.6889669190756865, -0.72479278722912, 0},
        {0.8207634412072763, -0.5712682150947924, 0},
        {0.9189578116202306, -0.3943558551133187, 0},
        {0.9795299412524945, -0.20129852008866028, 0},
        {1, 0, 0},
        {1.0, 0, 0.0},
        {0.9795299412524945, 0, 0.20129852008866006},
        {0.9189578116202306, 0, 0.39435585511331855},
        {0.8207634412072763, 0, 0.5712682150947923},
        {0.6889669190756866, 0, 0.72479278722912},
        {0.5289640103269624, 0, 0.8486442574947509},
        {0.3473052528448203, 0, 0.9377521321470804},
        {0.1514277775045767, 0, 0.9884683243281114},
        {-0.05064916883871264, 0, 0.9987165071710528},
        {-0.2506525322587204, 0, 0.9680771188662043},
        {-0.4403941515576344, 0, 0.8978045395707416},
        {-0.6121059825476629, 0, 0.7907757369376985},
        {-0.758758122692791, 0, 0.6513724827222223},
        {-0.8743466161445821, 0, 0.48530196253108104},
        {-0.9541392564000488, 0, 0.29936312297335804},
        {-0.9948693233918952, 0, 0.10116832198743228},
        {-0.9948693233918952, 0, -0.10116832198743204},
        {-0.9541392564000489, 0, -0.29936312297335776},
        {-0.8743466161445822, 0, -0.4853019625310808},
        {-0.7587581226927911, 0, -0.651372482722222},
        {-0.6121059825476627, 0, -0.7907757369376986},
        {-0.44039415155763423, 0, -0.8978045395707417},
        {-0.2506525322587205, 0, -0.9680771188662043},
        {-0.05064916883871266, 0, -0.9987165071710528},
        {0.15142777750457667, 0, -0.9884683243281114},
        {0.3473052528448203, 0, -0.9377521321470804},
        {0.5289640103269624, 0, -0.8486442574947509},
        {0.6889669190756865, 0, -0.72479278722912},
        {0.8207634412072763, 0, -0.5712682150947924},
        {0.9189578116202306, 0, -0.3943558551133187},
        {0.9795299412524945, 0, -0.20129852008866028},
        {1, 0, 0},
        {0, 1.0, 0.0},
        {0, 0.9795299412524945, 0.20129852008866006},
        {0, 0.9189578116202306, 0.39435585511331855},
        {0, 0.8207634412072763, 0.5712682150947923},
        {0, 0.6889669190756866, 0.72479278722912},
        {0, 0.5289640103269624, 0.8486442574947509},
        {0, 0.3473052528448203, 0.9377521321470804},
        {0, 0.1514277775045767, 0.9884683243281114},
        {0, -0.05064916883871264, 0.9987165071710528},
        {0, -0.2506525322587204, 0.9680771188662043},
        {0, -0.4403941515576344, 0.8978045395707416},
        {0, -0.6121059825476629, 0.7907757369376985},
        {0, -0.758758122692791, 0.6513724827222223},
        {0, -0.8743466161445821, 0.48530196253108104},
        {0, -0.9541392564000488, 0.29936312297335804},
        {0, -0.9948693233918952, 0.10116832198743228},
        {0, -0.9948693233918952, -0.10116832198743204},
        {0, -0.9541392564000489, -0.29936312297335776},
        {0, -0.8743466161445822, -0.4853019625310808},
        {0, -0.7587581226927911, -0.651372482722222},
        {0, -0.6121059825476627, -0.7907757369376986},
        {0, -0.44039415155763423, -0.8978045395707417},
        {0, -0.2506525322587205, -0.9680771188662043},
        {0, -0.05064916883871266, -0.9987165071710528},
        {0, 0.15142777750457667, -0.9884683243281114},
        {0, 0.3473052528448203, -0.9377521321470804},
        {0, 0.5289640103269624, -0.8486442574947509},
        {0, 0.6889669190756865, -0.72479278722912},
        {0, 0.8207634412072763, -0.5712682150947924},
        {0, 0.9189578116202306, -0.3943558551133187},
        {0, 0.9795299412524945, -0.20129852008866028},
        {0, 1, 0}
    };

    uint32 SphereFrameIndices[] =
    {
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
        11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20,
        21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30,
        31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40,
        41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50,
        51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60,
        61, 61, 62, 62, 63, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70,
        71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80,
        81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90,
        91, 91, 92, 92, 93, 93, 94, 94, 95
    };

    // 버텍스 버퍼 생성
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(SphereFrameVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    initData = {};
    initData.pSysMem = SphereFrameVertices;

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Sphere.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(SphereFrameIndices);

    initData.pSysMem = SphereFrameIndices;

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Sphere.Index);

    Resources.Primitives.Sphere.NumVertices = ARRAYSIZE(SphereFrameVertices);
    Resources.Primitives.Sphere.VertexStride = sizeof(FVector);
    Resources.Primitives.Sphere.NumIndices = ARRAYSIZE(SphereFrameIndices);



    ////////////////////////////////////
    // Cone 버퍼 생성
    // 0,0,0이 Apex
    // z=1이고, xy에서 r=1인 원이 밑변
    constexpr uint32 NumSegments = 32;
    TArray<FVector> ConeVertices;
    ConeVertices.Add({ 0.0f, 0.0f, 0.0f }); // Apex
    for (int i = 0; i < NumSegments; i++)
    {
        float angle = 2.0f * 3.1415926535897932f * i / (float)NumSegments;
        float x = cos(angle);
        float y = sin(angle);
        ConeVertices.Add({ x, y, 1.0f }); // Bottom
    }
    TArray<uint32> ConeIndices;
    constexpr uint32 vertexOffset0 = 1;
    // apex -> 밑면으로 가는 line
    for (int i = 0; i < NumSegments; i++)
    {
        ConeIndices.Add(0);
        ConeIndices.Add(vertexOffset0 + i);
    }
    // 밑변
    for (int i = 0; i < NumSegments; i++)
    {
        ConeIndices.Add(vertexOffset0 + i);
        ConeIndices.Add(vertexOffset0 + (i + 1) % NumSegments);
    }

    // cone을 덮는 sphere
    // xz plane
    float deltaAngle = 2.0f * 3.1415926535897932f / (float)NumSegments;
    float offsetAngle = deltaAngle * NumSegments / 8; // 45도 부터 시작
    for (int i = 0; i < NumSegments / 4 + 1; i++)
    {
        float angle = 2.0f * 3.1415926535897932f * i / (float)NumSegments + offsetAngle;
        float x = cos(angle) * sqrt(2.f);
        float z = sin(angle) * sqrt(2.f);
        ConeVertices.Add({ x, 0, z });
    }
    constexpr uint32 vertexOffset1 = NumSegments + vertexOffset0;
    for (int i = 0; i < NumSegments / 4; i++)
    {
        ConeIndices.Add(vertexOffset1 + i);
        ConeIndices.Add(vertexOffset1 + (i + 1));
    }
    // yz plane
    for (int i = 0; i < NumSegments / 4 + 1; i++)
    {
        float angle = 2.0f * 3.1415926535897932f * i / (float)NumSegments + offsetAngle;
        float y = cos(angle) * sqrt(2.f);
        float z = sin(angle) * sqrt(2.f);
        ConeVertices.Add({ 0, y, z });
    }
    constexpr uint32 vertexOffset2 = NumSegments / 4 + 1 + vertexOffset1;
    for (int i = 0; i < NumSegments / 4; i++)
    {
        ConeIndices.Add(vertexOffset2 + i);
        ConeIndices.Add(vertexOffset2 + (i + 1));
    }

    // 버텍스 버퍼 생성
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = ConeVertices.Num() * sizeof(FVector);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    initData = {};
    initData.pSysMem = ConeVertices.GetData();

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Cone.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = ConeIndices.Num() * sizeof(FVector);

    initData.pSysMem = ConeIndices.GetData();

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Cone.Index);

    Resources.Primitives.Cone.NumVertices = ConeVertices.Num();
    Resources.Primitives.Cone.VertexStride = sizeof(FVector);
    Resources.Primitives.Cone.NumIndices = ConeIndices.Num();
}
