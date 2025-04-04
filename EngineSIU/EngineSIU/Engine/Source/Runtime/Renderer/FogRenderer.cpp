#include "FogRenderer.h"
#include <d3dcompiler.h>
#include "Components/Quad.h"

void FFogRenderer::Initialize(FGraphicsDevice* device)
{
    Graphics = device;
    CreateShader();
    CreateConstantBuffer();
    CreateVertexBuffer();
    CreateIndexBuffer();
}

void FFogRenderer::UpdateConstant(FFogConstants Constant)
{
    if (FogConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        Graphics->DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
        {
            FFogConstants* constants = static_cast<FFogConstants*>(ConstantBufferMSR.pData);
            *constants = Constant;
        }
        Graphics->DeviceContext->Unmap(FogConstantBuffer, 0);
    }
}

void FFogRenderer::CreateShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    HRESULT hr = D3DCompileFromFile(L"Shaders/FogShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to compile vertex shader", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &VertexShader);

    hr = D3DCompileFromFile(L"Shaders/FogShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to compile pixel shader", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );

    Stride = sizeof(FVertexSimple);
    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FFogRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FFogConstants);
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FCone);

    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &FogConstantBuffer);
}

void FFogRenderer::CreateVertexBuffer()
{
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = sizeof(FVertexTexture) * 4;
    BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    BufferDesc.CPUAccessFlags = 0;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = quadVertices;

    Graphics->Device->CreateBuffer(&BufferDesc, &InitData, &FogVertexBuffer);
}

void FFogRenderer::CreateIndexBuffer()
{
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = sizeof(uint32) * 6;
    BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = quadIndices;

    Graphics->Device->CreateBuffer(&BufferDesc, &InitData, &FogIndexBuffer);
}

void FFogRenderer::Release()
{
    if (VertexShader) VertexShader->Release();
    if (PixelShader) PixelShader->Release();
    if (FogVertexBuffer) FogVertexBuffer->Release();
    if (FogIndexBuffer) FogIndexBuffer->Release();
    if (InputLayout) InputLayout->Release();
    if (FogConstantBuffer) FogConstantBuffer->Release();
}
