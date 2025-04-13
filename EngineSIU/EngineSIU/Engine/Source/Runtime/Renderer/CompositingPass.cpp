#include "CompositingPass.h"

#include <array>

#include "Define.h"
#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"

FCompositingPass::FCompositingPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FCompositingPass::~FCompositingPass()
{
    
}

void FCompositingPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;

    ShaderManager->AddVertexShader(L"Compositing", L"Shaders/CompositingShader.hlsl", "mainVS");
    ShaderManager->AddPixelShader(L"Compositing", L"Shaders/CompositingShader.hlsl", "mainPS");

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

void FCompositingPass::PrepareRender()
{
}

void FCompositingPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // TODO: 최종 결과 렌더하고, 결과 텍스처를 SRV를 통해 전달
    // Setup
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    if (!RenderTargetRHI)
    {
        return;
    }

    const EResourceType ResourceType = EResourceType::ERT_Compositing; 
    FViewportResources* ResourceRHI = Viewport->GetRenderTargetRHI()->GetResource(ResourceType);
    if (!ResourceRHI)
    {
        return;
    }

    Graphics->DeviceContext->OMSetRenderTargets(1, &ResourceRHI->RTV, nullptr);
    Graphics->DeviceContext->ClearDepthStencilView(RenderTargetRHI->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    Graphics->DeviceContext->ClearRenderTargetView(ResourceRHI->RTV, RenderTargetRHI->GetClearColor(ResourceType).data());

    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Sampler);

    // Update Constant Buffer
    FViewModeConstants ViewModeConstantData = {};
    ViewModeConstantData.ViewMode = Viewport->GetViewMode();
    BufferManager->UpdateConstantBuffer<FViewModeConstants>("FViewModeConstants", ViewModeConstantData);

    // Render
    Graphics->DeviceContext->VSSetShader(ShaderManager->GetVertexShaderByKey(L"Compositing"), nullptr, 1);
    Graphics->DeviceContext->PSSetShader(ShaderManager->GetPixelShaderByKey(L"Compositing"), nullptr, 1);
    Graphics->DeviceContext->Draw(6, 0);

    // Finish
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FCompositingPass::ClearRenderArr()
{
}
