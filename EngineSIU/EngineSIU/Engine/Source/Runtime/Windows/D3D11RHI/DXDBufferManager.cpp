#include "DXDBufferManager.h"



void FDXDBufferManager::Initialize(ID3D11Device* InDXDevice, ID3D11DeviceContext* InDXDeviceContext)
{
    DXDevice = InDXDevice;
    DXDeviceContext = InDXDeviceContext;
}

HRESULT FDXDBufferManager::CreateIndexBuffer(const FString& KeyName, const TArray<uint32>& indices, FIndexInfo& OutIndexInfo)
{
    if (KeyName != "" && IndexBufferPool.Contains(KeyName))
    {
        OutIndexInfo = IndexBufferPool[KeyName];
        return S_OK;
    }

    ID3D11Buffer* NewBuffer = nullptr;

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(uint32) * static_cast<uint32>(indices.Num());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.GetData();

    HRESULT hr = DXDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutIndexInfo.NumIndices = static_cast<uint32>(indices.Num());
    OutIndexInfo.IndexBuffer = NewBuffer;

    IndexBufferPool[KeyName] = FIndexInfo(static_cast<uint32>(indices.Num()), NewBuffer);

    return S_OK;
}

HRESULT FDXDBufferManager::CreateTextureVertexBuffer(const FWString& KeyName, const TArray<FVertexTexture>& vertices, FVertexInfo& OutVertexInfo)
{
    if (!KeyName.empty() && TextAtlasBufferPool.Contains(KeyName))
    {
        OutVertexInfo = TextAtlasBufferPool.Find(KeyName)->VertexInfo;
        return S_OK;
    }

    ID3D11Buffer* NewBuffer = nullptr;
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(FVertexTexture) * static_cast<UINT>(vertices.Num());
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexInitData = {};
    vertexInitData.pSysMem = vertices.GetData();

    HRESULT hr = DXDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutVertexInfo.VertexBuffer = NewBuffer;
    OutVertexInfo.NumVertices = static_cast<uint32>(vertices.Num());

    TextAtlasBufferPool[KeyName] = FBufferInfo();
    TextAtlasBufferPool[KeyName].VertexInfo = OutVertexInfo;
    return S_OK;
}

HRESULT FDXDBufferManager::CreateTextureIndexBuffer(const FWString& KeyName, const TArray<uint32>& indices, FIndexInfo& OutIndexInfo)
{
    if (KeyName.empty() && TextAtlasBufferPool.Contains(KeyName))
    {
        OutIndexInfo = TextAtlasBufferPool.Find(KeyName)->IndexInfo;
        return S_OK;
    }

    ID3D11Buffer* NewBuffer = nullptr;

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(uint32) * static_cast<uint32>(indices.Num());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.GetData();

    HRESULT hr = DXDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutIndexInfo.NumIndices = static_cast<uint32>(indices.Num());
    
    OutIndexInfo.IndexBuffer = NewBuffer;

    TextAtlasBufferPool[KeyName] = FBufferInfo();

    TextAtlasBufferPool[KeyName].IndexInfo = FIndexInfo(static_cast<uint32>(indices.Num()), NewBuffer);

    return S_OK;
}

void FDXDBufferManager::GetStartUV(wchar_t ch, float& outStartU, float& outStartV, int ColumnCount)
{
    int StartU = 0, StartV = 0, offset = -1;
    if (ch == L' ') {
        outStartU = 0;
        outStartV = 0;
        return;
    }
    else if (ch >= L'A' && ch <= L'Z') {
        StartU = 11;
        StartV = 0;
        offset = ch - L'A';
    }
    else if (ch >= L'a' && ch <= L'z') {
        StartU = 37;
        StartV = 0;
        offset = ch - L'a';
    }
    else if (ch >= L'0' && ch <= L'9') {
        StartU = 1;
        StartV = 0;
        offset = ch - L'0';
    }
    else {
        // 기본 처리: 없으면 (0,0) 반환
        outStartU = 0;
        outStartV = 0;
        return;
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;
    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}
HRESULT FDXDBufferManager::CreateASCIITextBuffer(const FWString& Text, FTexture& Texture, uint16_t RowCount, uint16_t ColumnCount,int quadWidth,  FBufferInfo& OutBufferInfo, float WidthOffset, float HeightOffset)
{
    if (TextAtlasBufferPool.Contains(Text))
    {
        OutBufferInfo = TextAtlasBufferPool[Text];
        return S_OK;
    }

    TArray<FVertexTexture> Vertices;
    TArray<uint32> Indices;
    uint32 baseIndex = 0;

    // 텍스처 아틀라스의 셀 크기를 계산 (가정: BitmapWidth, BitmapHeight는 미리 정해진 값)
    uint32 BitmapWidth = 256;  // 예시 값 (실제 값으로 대체)
    uint32 BitmapHeight = 256; // 예시 값 (실제 값으로 대체)
    float CellWidth = float(BitmapWidth) / ColumnCount;
    float CellHeight = float(BitmapHeight) / RowCount;
    float nTexelUOffset = CellWidth / BitmapWidth;
    float nTexelVOffset = CellHeight / BitmapHeight;

    // 각 문자마다 사각형(쿼드)를 생성
    for (int i = 0; i < Text.size(); i++)
    {
        // 기본 사각형 정점 (UV는 [0,1] 범위)
        FVertexTexture leftUP = { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f };
        FVertexTexture rightUP = { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f };
        FVertexTexture leftDown = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f };
        FVertexTexture rightDown = { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f };

        // 각 UV에 텍셀 오프셋 곱하기
        rightUP.u *= nTexelUOffset;
        leftDown.v *= nTexelVOffset;
        rightDown.u *= nTexelUOffset;
        rightDown.v *= nTexelVOffset;

        // 각 문자마다 quadWidth 만큼 x 좌표를 이동시킴
        leftUP.x += quadWidth * i;
        rightUP.x += quadWidth * i;
        leftDown.x += quadWidth * i;
        rightDown.x += quadWidth * i;

        float startU = 0.0f;
        float startV = 0.0f;
        // 문자에 따른 UV 시작 오프셋 계산 (ColumnCount를 전달)
        GetStartUV(Text[i], startU, startV, ColumnCount);

        // 계산된 시작 오프셋을 적용
        leftUP.u += nTexelUOffset * startU;
        leftUP.v += nTexelVOffset * startV;
        rightUP.u += nTexelUOffset * startU;
        rightUP.v += nTexelVOffset * startV;
        leftDown.u += nTexelUOffset * startU;
        leftDown.v += nTexelVOffset * startV;
        rightDown.u += nTexelUOffset * startU;
        rightDown.v += nTexelVOffset * startV;

        // 두 삼각형으로 쿼드 구성: (leftUP, rightUP, leftDown) 및 (rightUP, rightDown, leftDown)
        Vertices.Add(leftUP);
        Vertices.Add(rightUP);
        Vertices.Add(leftDown);
        Vertices.Add(rightUP);
        Vertices.Add(rightDown);
        Vertices.Add(leftDown);

        // 인덱스 추가
        Indices.Add(baseIndex + 0);
        Indices.Add(baseIndex + 1);
        Indices.Add(baseIndex + 2);
        Indices.Add(baseIndex + 3);
        Indices.Add(baseIndex + 4);
        Indices.Add(baseIndex + 5);
        baseIndex += 6;
    }

    UINT byteWidth = static_cast<UINT>(Vertices.Num() * sizeof(FVertexTexture));

    // 버텍스 버퍼 생성
    HRESULT hr = CreateTextureVertexBuffer(Text, Vertices, OutBufferInfo.VertexInfo);
    
    if (FAILED(hr))
        return hr;

    // 인덱스 버퍼 생성
    hr = CreateTextureIndexBuffer(Text, Indices, OutBufferInfo.IndexInfo);
   
    if (FAILED(hr))
        return hr;

   
    return S_OK;
}

void FDXDBufferManager::ReleaseBuffers()
{
    for (auto& Pair : VertexBufferPool)
    {
        if (Pair.Value.VertexBuffer)
        {
            Pair.Value.VertexBuffer->Release();
            Pair.Value.VertexBuffer = nullptr;
        }
    }
    VertexBufferPool.Empty();

    for (auto& Pair : IndexBufferPool)
    {
        if (Pair.Value.IndexBuffer)
        {
            Pair.Value.IndexBuffer->Release();
            Pair.Value.IndexBuffer = nullptr;
        }
    }
    IndexBufferPool.Empty();

    TextAtlasBufferPool.Empty();
}

void FDXDBufferManager::ReleaseConstantBuffer()
{
    for (auto& Pair : ConstantBufferPool)
    {
        if (Pair.Value)
        {
            Pair.Value->Release();
            Pair.Value = nullptr;
        }
    }
    VertexBufferPool.Empty();
    
}

FVertexInfo FDXDBufferManager::GetVertexBuffer(const FString& InName) const
{
    if (VertexBufferPool.Contains(InName))
    {
        return VertexBufferPool[InName];
    }
    return FVertexInfo();
}

FIndexInfo FDXDBufferManager::GetIndexBuffer(const FString& InName) const
{
    if (IndexBufferPool.Contains(InName))
    {
        return IndexBufferPool[InName];
    }
    return FIndexInfo();
}

FVertexInfo FDXDBufferManager::GetTextVertexBuffer(const FWString& InName) const
{
    if (TextAtlasBufferPool.Contains(InName))
    {
        return TextAtlasBufferPool[InName].VertexInfo;
    }
    return FVertexInfo();
}

FVertexInfo FDXDBufferManager::GetTextureIndexBuffer(const FWString& InName) const
{
    if (TextAtlasBufferPool.Contains(InName))
    {
        return TextAtlasBufferPool[InName].VertexInfo;
    }
    return FVertexInfo();
}

ID3D11Buffer* FDXDBufferManager::GetConstantBuffer(const FString& InName) const
{
    if (ConstantBufferPool.Contains(InName))
    {
        return ConstantBufferPool[InName];
    }
    return nullptr;
}
