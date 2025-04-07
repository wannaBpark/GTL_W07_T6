#include "DXDBufferManager.h"

void FDXDBufferManager::Initialize(ID3D11Device* InDXDevice, ID3D11DeviceContext* InDXDeviceContext)
{
    DXDevice = InDXDevice;
    DXDeviceContext = InDXDeviceContext;
}

HRESULT FDXDBufferManager::CreateIndexBuffer(const FString& KeyName, const TArray<uint32>& indices, FIndexInfo& OutIndexInfo)
{
    if (!KeyName.IsEmpty() && IndexBufferPool.Contains(KeyName))
    {
        OutIndexInfo = IndexBufferPool[KeyName];
        return S_OK;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(uint32) * static_cast<uint32>(indices.Num());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutIndexInfo.NumIndices = static_cast<uint32>(indices.Num());
    OutIndexInfo.IndexBuffer = NewBuffer;
    IndexBufferPool.Add(KeyName, FIndexInfo(static_cast<uint32>(indices.Num()), NewBuffer));

    return S_OK;
}

HRESULT FDXDBufferManager::CreateTextureVertexBuffer(const FWString& KeyName, const TArray<FVertexTexture>& vertices, FVertexInfo& OutVertexInfo)
{
    if (!KeyName.empty() && TextAtlasBufferPool.Contains(KeyName))
    {
        OutVertexInfo = TextAtlasBufferPool[KeyName].VertexInfo;
        return S_OK;
    }

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(FVertexTexture) * static_cast<UINT>(vertices.Num());
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexInitData = {};
    vertexInitData.pSysMem = vertices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutVertexInfo.VertexBuffer = NewBuffer;
    OutVertexInfo.NumVertices = static_cast<uint32>(vertices.Num());

    FBufferInfo bufferInfo;
    bufferInfo.VertexInfo = OutVertexInfo;
    TextAtlasBufferPool.Add(KeyName, bufferInfo);

    return S_OK;
}

HRESULT FDXDBufferManager::CreateTextureIndexBuffer(const FWString& KeyName, const TArray<uint32>& indices, FIndexInfo& OutIndexInfo)
{
    if (!KeyName.empty() && TextAtlasBufferPool.Contains(KeyName))
    {
        OutIndexInfo = TextAtlasBufferPool[KeyName].IndexInfo;
        return S_OK;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(uint32) * static_cast<UINT>(indices.Num());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutIndexInfo.NumIndices = static_cast<uint32>(indices.Num());
    OutIndexInfo.IndexBuffer = NewBuffer;

    FBufferInfo bufferInfo;
    bufferInfo.IndexInfo = FIndexInfo(static_cast<uint32>(indices.Num()), NewBuffer);
    TextAtlasBufferPool.Add(KeyName, bufferInfo);

    return S_OK;
}

void FDXDBufferManager::GetStartUV(wchar_t ch, float& outStartU, float& outStartV, int ColumnCount)
{
    int StartU = 0, StartV = 0, offset = -1;
    if (ch == L' ')
    {
        outStartU = 0;
        outStartV = 0;
        return;
    }
    else if (ch >= L'A' && ch <= L'Z')
    {
        StartU = 11;
        StartV = 0;
        offset = ch - L'A';
    }
    else if (ch >= L'a' && ch <= L'z')
    {
        StartU = 37;
        StartV = 0;
        offset = ch - L'a';
    }
    else if (ch >= L'0' && ch <= L'9')
    {
        StartU = 1;
        StartV = 0;
        offset = ch - L'0';
    }
    else
    {
        outStartU = 0;
        outStartV = 0;
        return;
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;
    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}

HRESULT FDXDBufferManager::CreateASCIITextBuffer(const FWString& Text, FTexture& Texture, uint16_t RowCount, uint16_t ColumnCount, int quadWidth, FBufferInfo& OutBufferInfo, float WidthOffset, float HeightOffset)
{
    if (TextAtlasBufferPool.Contains(Text))
    {
        OutBufferInfo = TextAtlasBufferPool[Text];
        return S_OK;
    }

    TArray<FVertexTexture> Vertices;
    TArray<uint32> Indices;
    uint32 baseIndex = 0;

    // Bitmap 크기 (예제 값)
    uint32 BitmapWidth = 256;
    uint32 BitmapHeight = 256;
    float CellWidth = float(BitmapWidth) / ColumnCount;
    float CellHeight = float(BitmapHeight) / RowCount;
    float nTexelUOffset = CellWidth / BitmapWidth;
    float nTexelVOffset = CellHeight / BitmapHeight;

    for (size_t i = 0; i < Text.size(); i++)
    {
        FVertexTexture leftUP = { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f };
        FVertexTexture rightUP = { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f };
        FVertexTexture leftDown = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f };
        FVertexTexture rightDown = { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f };

        rightUP.u *= nTexelUOffset;
        leftDown.v *= nTexelVOffset;
        rightDown.u *= nTexelUOffset;
        rightDown.v *= nTexelVOffset;

        leftUP.x += quadWidth * i;
        rightUP.x += quadWidth * i;
        leftDown.x += quadWidth * i;
        rightDown.x += quadWidth * i;

        float startU = 0.0f, startV = 0.0f;
        GetStartUV(Text[i], startU, startV, ColumnCount);

        leftUP.u += nTexelUOffset * startU;
        leftUP.v += nTexelVOffset * startV;
        rightUP.u += nTexelUOffset * startU;
        rightUP.v += nTexelVOffset * startV;
        leftDown.u += nTexelUOffset * startU;
        leftDown.v += nTexelVOffset * startV;
        rightDown.u += nTexelUOffset * startU;
        rightDown.v += nTexelVOffset * startV;

        Vertices.Add(leftUP);
        Vertices.Add(rightUP);
        Vertices.Add(leftDown);
        Vertices.Add(rightUP);
        Vertices.Add(rightDown);
        Vertices.Add(leftDown);

        Indices.Add(baseIndex + 0);
        Indices.Add(baseIndex + 1);
        Indices.Add(baseIndex + 2);
        Indices.Add(baseIndex + 3);
        Indices.Add(baseIndex + 4);
        Indices.Add(baseIndex + 5);
        baseIndex += 6;
    }

    HRESULT hr = CreateTextureVertexBuffer(Text, Vertices, OutBufferInfo.VertexInfo);
    if (FAILED(hr))
        return hr;

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
    ConstantBufferPool.Empty();
}
void FDXDBufferManager::BindConstantBuffers(const TArray<FString>& Keys, UINT StartSlot, EShaderStage Stage) const
{
    const int Count = Keys.Num();
    TArray<ID3D11Buffer*> Buffers;
    Buffers.Reserve(Count);
    for (const FString& Key : Keys)
    {
        ID3D11Buffer* Buffer = GetConstantBuffer(Key);
        Buffers.Add(Buffer);
    }

    if (Stage == EShaderStage::Vertex)
        DXDeviceContext->VSSetConstantBuffers(StartSlot, Count, Buffers.GetData());
    else if (Stage == EShaderStage::Pixel)
        DXDeviceContext->PSSetConstantBuffers(StartSlot, Count, Buffers.GetData());
}

void FDXDBufferManager::BindConstantBuffer(const FString& Key, UINT StartSlot, EShaderStage Stage) const
{
    ID3D11Buffer* Buffer = GetConstantBuffer(Key);
    if (Stage == EShaderStage::Vertex)
        DXDeviceContext->VSSetConstantBuffers(StartSlot, 1, &Buffer);
    else if (Stage == EShaderStage::Pixel)
        DXDeviceContext->PSSetConstantBuffers(StartSlot, 1, &Buffer);
}

FVertexInfo FDXDBufferManager::GetVertexBuffer(const FString& InName) const
{
    if (VertexBufferPool.Contains(InName))
        return VertexBufferPool[InName];
    return FVertexInfo();
}

FIndexInfo FDXDBufferManager::GetIndexBuffer(const FString& InName) const
{
    if (IndexBufferPool.Contains(InName))
        return IndexBufferPool[InName];
    return FIndexInfo();
}

FVertexInfo FDXDBufferManager::GetTextVertexBuffer(const FWString& InName) const
{
    if (TextAtlasBufferPool.Contains(InName))
        return TextAtlasBufferPool[InName].VertexInfo;
    return FVertexInfo();
}

FVertexInfo FDXDBufferManager::GetTextureIndexBuffer(const FWString& InName) const
{
    if (TextAtlasBufferPool.Contains(InName))
        return TextAtlasBufferPool[InName].VertexInfo;
    return FVertexInfo();
}

ID3D11Buffer* FDXDBufferManager::GetConstantBuffer(const FString& InName) const
{
    if (ConstantBufferPool.Contains(InName))
        return ConstantBufferPool[InName];
    return nullptr;
}
