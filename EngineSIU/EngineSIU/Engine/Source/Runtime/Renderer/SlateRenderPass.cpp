#include "SlateRenderPass.h"

#include "UnrealClient.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorViewportClient.h"

FSlateRenderPass::FSlateRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FSlateRenderPass::~FSlateRenderPass()
{
}

void FSlateRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();

    HRESULT hr = S_OK;
    hr = BufferManager->CreateBufferGeneric<FSlateTransform>("FSlateTransform", nullptr, sizeof(FSlateTransform), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    if (FAILED(hr))
    {
        return;
    }

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    Graphics->Device->CreateSamplerState(&SamplerDesc, &Sampler);
}

void FSlateRenderPass::PrepareRender()
{
}

void FSlateRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FRect Rect = Viewport->GetViewport()->GetRect();

    // 버퍼 업데이트
    FSlateTransform Transform;

    Transform.Scale = FVector2D(1.f, 1.f);
    Transform.Offset = FVector2D(0.f, 0.f);
    
    BufferManager->UpdateConstantBuffer<FSlateTransform>("FSlateTransform", Transform);

    // 렌더 타겟을 백버퍼로 지정
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->BackBufferRTV, nullptr);
    Graphics->DeviceContext->RSSetViewports(1, &Graphics->Viewport);

    // 렌더 준비
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    FViewportResources* Resource = RenderTargetRHI->GetResources().Find(EResourceType::ERT_Compositing);

    Graphics->DeviceContext->PSSetShaderResources(120, 1, &Resource->SRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Sampler);

    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"SlateShader");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"SlateShader");
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(nullptr);

    // 렌더
    Graphics->DeviceContext->Draw(6, 0);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(100, 1, NullSRV);
}

void FSlateRenderPass::ClearRenderArr()
{
}

void FSlateRenderPass::CreateShader()
{
    HRESULT hr = S_OK;
    hr = ShaderManager->AddVertexShader(L"SlateShader", L"Shaders/SlateShader.hlsl", "mainVS");
    if (FAILED(hr))
    {
        return;
    }
    
    hr = ShaderManager->AddPixelShader(L"SlateShader", L"Shaders/SlateShader.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }
}

