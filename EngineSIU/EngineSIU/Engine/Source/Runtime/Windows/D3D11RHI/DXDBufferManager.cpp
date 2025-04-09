#include "DXDBufferManager.h"

void FDXDBufferManager::Initialize(ID3D11Device* InDXDevice, ID3D11DeviceContext* InDXDeviceContext)
{
    DXDevice = InDXDevice;
    DXDeviceContext = InDXDeviceContext;
    CreateQuadBuffer();
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

ID3D11Buffer* FDXDBufferManager::GetConstantBuffer(const FString& InName) const
{
    if (ConstantBufferPool.Contains(InName))
        return ConstantBufferPool[InName];
    return nullptr;
}

void FDXDBufferManager::CreateQuadBuffer()
{
    TArray<QuadVertex> Vertices =
    {
        { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
    };

    FVertexInfo VertexInfo;
    CreateVertexBuffer(TEXT("QuadBuffer"), Vertices, VertexInfo);

    TArray<short> Indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    FIndexInfo IndexInfo;
    CreateIndexBuffer(TEXT("QuadBuffer"), Indices, IndexInfo);
}

void FDXDBufferManager::GetQuadBuffer(FVertexInfo& OutVertexInfo, FIndexInfo& OutIndexInfo)
{
    OutVertexInfo = GetVertexBuffer(TEXT("QuadBuffer"));
    OutIndexInfo = GetIndexBuffer(TEXT("QuadBuffer"));
}
