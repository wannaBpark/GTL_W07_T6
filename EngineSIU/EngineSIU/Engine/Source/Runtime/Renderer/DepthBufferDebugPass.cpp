#include "DepthBufferDebugPass.h"

#include <array>

#include "UnrealClient.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "UnrealEd/EditorViewportClient.h"

struct FViewportResources;

FDepthBufferDebugPass::FDepthBufferDebugPass()
    : Graphics(nullptr)
    , BufferManager(nullptr)
    , ShaderManager(nullptr)
    , VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , DepthSampler(nullptr)
{
}

FDepthBufferDebugPass::~FDepthBufferDebugPass()
{
    if (VertexShader) { VertexShader->Release(); VertexShader = nullptr; }
    if (PixelShader) { PixelShader->Release(); PixelShader = nullptr; }
    if (InputLayout) { InputLayout->Release(); InputLayout = nullptr; }
}

void FDepthBufferDebugPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;
    ShaderManager = InShaderManager;
    
    CreateShader();
}

void FDepthBufferDebugPass::CreateShader()
{
    // 입력 레이아웃 정의: POSITION과 TEXCOORD
    D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // 정점 셰이더 및 입력 레이아웃 생성
    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"DepthDebugVertexShader",
        L"Shaders/DepthBufferVertexShader.hlsl",
        "mainVS",
        InputLayoutDesc,
        ARRAYSIZE(InputLayoutDesc)
    );

    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(
        L"DepthDebugPixelShader",
        L"Shaders/DepthBufferPixelShader.hlsl",
        "mainPS"
    );

    // 생성된 셰이더와 입력 레이아웃 획득
    VertexShader = ShaderManager->GetVertexShaderByKey(L"DepthDebugVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"DepthDebugPixelShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"DepthDebugVertexShader");

    // Sampler
    D3D11_SAMPLER_DESC SamplerDesc;
    ZeroMemory(&SamplerDesc, sizeof(SamplerDesc));
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&SamplerDesc, &DepthSampler);
}

void FDepthBufferDebugPass::PrepareRenderState()
{
    // 셰이더 설정
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    
    Graphics->DeviceContext->PSSetSamplers(0, 1, &DepthSampler);
}

void FDepthBufferDebugPass::UpdateScreenConstant(const D3D11_VIEWPORT& viewport)
{
    float sw = float(screenWidth);
    float sh = float(screenHeight);

    FScreenConstants sc;
    sc.ScreenSize = { sw, sh };
    sc.UVOffset = { viewport.TopLeftX / sw, viewport.TopLeftY / sh };
    sc.UVScale = { viewport.Width / sw, viewport.Height / sh };
    sc.Padding = { 0.0f, 0.0f };

    BufferManager->UpdateConstantBuffer(TEXT("FScreenConstants"), sc);
    BufferManager->BindConstantBuffer(TEXT("FScreenConstants"), 0, EShaderStage::Pixel);
}

void FDepthBufferDebugPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    
}
