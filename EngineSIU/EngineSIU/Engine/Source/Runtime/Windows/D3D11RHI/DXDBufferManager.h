#pragma once
#define _TCHAR_DEFINED
#include "Define.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Container/String.h"
#include "Container/Array.h"
#include "Container/Map.h"
#include "Engine/Texture.h"
#include "GraphicDevice.h"

// ShaderStage 열거형 (혹은 별도의 구분 방식)
enum class EShaderStage
{
    Vertex,
    Pixel
};

class FDXDBufferManager
{

public:
    FDXDBufferManager() = default;
    void Initialize(ID3D11Device* DXDevice, ID3D11DeviceContext* DXDeviceContext);

    template<typename T>
    HRESULT CreateVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo);

    HRESULT CreateIndexBuffer(const FString& KeyName, const TArray<uint32>& indices, FIndexInfo& OutIndexInfo);

    HRESULT CreateTextureVertexBuffer(const FWString& KeyName, const TArray<FVertexTexture>& vertices, FVertexInfo& OutVertexInfo);

    HRESULT CreateTextureIndexBuffer(const FWString& KeyName, const TArray<uint32>& indices, FIndexInfo& OutIndexInfo);

    void GetStartUV(wchar_t ch, float& outStartU, float& outStartV, int ColumnCount);

    HRESULT CreateASCIITextBuffer(const FWString& Text, FTexture& Texture, uint16_t RowCount, uint16_t ColumnCount, int quadWidth, FBufferInfo& OutBufferInfo, float WidthOffset, float HeightOffset);

    void ReleaseBuffers();
    void ReleaseConstantBuffer();

    template<typename T>
    HRESULT CreateBufferGeneric(const FString& KeyName, T* data, UINT byteWidth, UINT bindFlags, D3D11_USAGE usage, UINT cpuAccessFlags);

    template<typename T>
    HRESULT CreateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo);

    template<typename T>
    void UpdateConstantBuffer(const FString& key, const T& data) const;

    template<typename T>
    void UpdateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices) const;

    template<typename T>
    static void SafeRelease(T*& comObject);

    FVertexInfo GetVertexBuffer(const FString& InName)const;

    FIndexInfo GetIndexBuffer(const FString& InName) const;

    FVertexInfo GetTextVertexBuffer(const FWString& InName) const;

    FVertexInfo GetTextureIndexBuffer(const FWString& InName) const;

    ID3D11Buffer* GetConstantBuffer(const FString& InName) const;

    void BindConstantBuffers(const TArray<FString>& Keys, UINT StartSlot, EShaderStage Stage) const
    {
        const int Count = Keys.Num();

        // 키 목록에 해당하는 버퍼 배열 생성
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

    void BindConstantBuffer(const FString& Key, UINT StartSlot, EShaderStage Stage) const
    {
        ID3D11Buffer* Buffer = GetConstantBuffer(Key);
        if (Stage == EShaderStage::Vertex)
            DXDeviceContext->VSSetConstantBuffers(StartSlot, 1, &Buffer);
        else if (Stage == EShaderStage::Pixel)
            DXDeviceContext->PSSetConstantBuffers(StartSlot, 1, &Buffer);
    }
        
private:
    inline UINT Align16(UINT size)
    {
        return (size + 15) & ~15;
    }
private:

    ID3D11Device* DXDevice;

    ID3D11DeviceContext* DXDeviceContext;

    TMap<FString, FVertexInfo> VertexBufferPool;
    TMap<FString, FIndexInfo> IndexBufferPool;

    TMap<FWString, FBufferInfo> TextAtlasBufferPool;

    TMap<FString, ID3D11Buffer*> ConstantBufferPool;
};

template<typename T>
inline HRESULT FDXDBufferManager::CreateVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo)
{
    if (KeyName != TEXT("") && VertexBufferPool.Contains(KeyName))
    {
        OutVertexInfo = VertexBufferPool[KeyName];
        return S_OK;
    }

    ID3D11Buffer* NewVertexBuffer;
    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(T) * static_cast<uint32>(vertices.Num());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.GetData();

    HRESULT hr = DXDevice->CreateBuffer(&bufferDesc, &initData, &NewVertexBuffer);
    if (FAILED(hr))
        return hr;

    OutVertexInfo.NumVertices = static_cast<uint32>(vertices.Num());
    OutVertexInfo.VertexBuffer = NewVertexBuffer;

    VertexBufferPool[KeyName] = OutVertexInfo;
    return S_OK;
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo)
{

    if (VertexBufferPool.Contains(KeyName))
    {
        OutVertexInfo = VertexBufferPool[KeyName];
        return S_OK;
    }

    ID3D11Buffer* NewVertexBuffer;
    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(T) * static_cast<uint32>(vertices.Num());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.GetData();

    HRESULT hr = DXDevice->CreateBuffer(&bufferDesc, &initData, &NewVertexBuffer);
    if (FAILED(hr))
        return hr;

    OutVertexInfo.NumVertices = static_cast<uint32>(vertices.Num());
    OutVertexInfo.VertexBuffer = NewVertexBuffer;

    VertexBufferPool[KeyName] = OutVertexInfo;
    return S_OK;
}

template<typename T>
HRESULT FDXDBufferManager::CreateBufferGeneric(const FString& KeyName, T* data, UINT byteWidth, UINT bindFlags, D3D11_USAGE usage, UINT cpuAccessFlags)
{
    byteWidth = Align16(byteWidth);

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = byteWidth;
    desc.Usage = usage;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = cpuAccessFlags;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;

    ID3D11Buffer* buffer = nullptr;

    HRESULT hr = DXDevice->CreateBuffer(&desc, data ? &initData : nullptr, &buffer);

    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, "Error Create Constant Buffer!");
        return hr;
    }

    ConstantBufferPool[KeyName] = buffer;

    return S_OK;

}

template<typename T>
void FDXDBufferManager::UpdateConstantBuffer(const FString& key, const T& data) const
{
    ID3D11Buffer* buffer = GetConstantBuffer(key);
    if (!buffer)
    {
        UE_LOG(LogLevel::Error, "UpdateConstantBuffer 호출: 키 %s에 해당하는 buffer가 없습니다.", *key);
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = DXDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, "Buffer Map 실패, HRESULT: 0x%X", hr);
        return;
    }

    memcpy(mappedResource.pData, &data, sizeof(T));
    DXDeviceContext->Unmap(buffer, 0);
}
template<typename T>
void FDXDBufferManager::UpdateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices) const
{
    // 1) 풀에서 버퍼 정보 조회
    FVertexInfo vbInfo;
    if (!VertexBufferPool.Contains(KeyName))
    {
        UE_LOG(LogLevel::Error, "UpdateDynamicVertexBuffer 호출: 키 %s에 해당하는 버텍스 버퍼가 없습니다.", *KeyName);
        return;
    }
    vbInfo = VertexBufferPool[KeyName];

    // 2) 맵핑
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = DXDeviceContext->Map(vbInfo.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, "VertexBuffer Map 실패, HRESULT: 0x%X", hr);
        return;
    }

    // 3) 데이터 복사
    memcpy(mapped.pData, vertices.GetData(), sizeof(T) * vertices.Num());

    // 4) 언맵
    DXDeviceContext->Unmap(vbInfo.VertexBuffer, 0);
}

template<typename T>
inline void FDXDBufferManager::SafeRelease(T*& comObject)
{
    if (comObject)
    {
        comObject->Release();
        comObject = nullptr;
    }
}
