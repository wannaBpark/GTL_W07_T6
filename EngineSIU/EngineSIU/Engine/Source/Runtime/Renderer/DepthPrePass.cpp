#include "DepthPrePass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Renderer/Renderer.h"
#include "DepthBufferDebugPass.h"
#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

FDepthPrePass::FDepthPrePass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
    , DepthStencilState_OnlyWrite(nullptr)
    , DepthStencilBuffer(nullptr)
    , DepthStencilView(nullptr)
    , DepthSRV(nullptr)
{
}

FDepthPrePass::~FDepthPrePass()
{
    Release();
}

void FDepthPrePass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;

    CreateDepthStencilState();
}

void FDepthPrePass::PrepareRender()
{
    // A. 컬러 쓰기 비활성화 (깊이만 기록)
    FLOAT zeroColor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    Graphics->DeviceContext->OMSetBlendState(nullptr, zeroColor, 0xFFFFFFFF); // 또는 Custom DisableBlendState
    // B. Depth-only Rasterizer & DepthStencil 설정
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStencilState_OnlyWrite, 0);  // 깊이만 기록, 테스트 ON

    // C. 컬러 렌더 타겟 없음, 깊이만
    ID3D11RenderTargetView* nullRTV = nullptr;
    Graphics->DeviceContext->OMSetRenderTargets(1, &nullRTV, DepthStencilView); // ← 깊이 전용
}

void FDepthPrePass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FDepthPrePass::ClearRenderArr()
{
    Graphics->DeviceContext->OMSetRenderTargets(0,nullptr, nullptr);
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, Graphics->DepthStencilView);
}

void FDepthPrePass::CreateDepthStencilState()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = FALSE;
    Graphics->Device->CreateDepthStencilState(&dsDesc, &DepthStencilState_OnlyWrite);

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = Graphics->screenWidth;
    depthDesc.Height = Graphics->screenHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for both DSV + SRV
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.CPUAccessFlags = 0;                     // CPU 접근 방식 설정
    depthDesc.MiscFlags = 0;                          // 기타 플래그 설정
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = Graphics->Device->CreateTexture2D(&depthDesc, nullptr, &DepthStencilBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create depth stencil buffer!"));
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    hr = Graphics->Device->CreateDepthStencilView(DepthStencilBuffer, &dsvDesc, &DepthStencilView);

    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create depth stencil view!"));
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = Graphics->Device->CreateShaderResourceView(DepthStencilBuffer, &srvDesc, &DepthSRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create depth SRV!"));
    }
}

void FDepthPrePass::Release()
{
    SAFE_RELEASE(DepthStencilState_OnlyWrite);
    SAFE_RELEASE(DepthStencilBuffer);
    SAFE_RELEASE(DepthStencilView);
    SAFE_RELEASE(DepthSRV);
}

void FDepthPrePass::ResizeDepthStencil()
{
    Release();
    CreateDepthStencilState();
}
