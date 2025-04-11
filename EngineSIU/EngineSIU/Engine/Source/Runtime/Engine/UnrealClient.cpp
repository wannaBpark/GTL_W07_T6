#include "UnrealClient.h"

#include "EngineLoop.h"

FRenderTarget::FRenderTarget()
{
}

FRenderTarget::~FRenderTarget()
{
    Release();
}

void FRenderTarget::Initialize(uint32 InWidth, uint32 InHeight)
{
    HRESULT hr = S_OK;
    hr = CreateSceneResources(InWidth, InHeight);
    if (FAILED(hr))
    {
        return;
    }

    hr = CreateDepthStencilResources(InWidth, InHeight);
    if (FAILED(hr))
    {
        return;
    }
}

void FRenderTarget::Resize(uint32 NewWidth, uint32 NewHeight)
{
}

void FRenderTarget::Release()
{
    if (SceneTexture)
    {
        SceneTexture->Release();
        SceneTexture = nullptr;
    }
    if (SceneRTV)
    {
        SceneRTV->Release();
        SceneRTV = nullptr;
    }
    if (SceneSRV)
    {
        SceneSRV->Release();
        SceneSRV = nullptr;
    }
    if (DepthStencilTexture)
    {
        DepthStencilTexture->Release();
        DepthStencilTexture = nullptr;
    }
    if (DepthStencilView)
    {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }
    if (DepthStencilSRV)
    {
        DepthStencilSRV->Release();
        DepthStencilSRV = nullptr;
    }
}

HRESULT FRenderTarget::CreateSceneResources(uint32 InWidth, uint32 InHeight)
{
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC SceneTextureDesc = {};
    SceneTextureDesc.Width = InWidth;
    SceneTextureDesc.Height = InHeight;
    SceneTextureDesc.MipLevels = 1;
    SceneTextureDesc.ArraySize = 1;
    SceneTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SceneTextureDesc.SampleDesc.Count = 1;
    SceneTextureDesc.SampleDesc.Quality = 0;
    SceneTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    SceneTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    SceneTextureDesc.CPUAccessFlags = 0;
    SceneTextureDesc.MiscFlags = 0;
    SceneTexture = FEngineLoop::GraphicDevice.CreateTexture2D(SceneTextureDesc, nullptr);

    D3D11_RENDER_TARGET_VIEW_DESC SceneRTVDesc = {};
    SceneRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: srgb 옵션 고려해보기
    SceneRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = FEngineLoop::GraphicDevice.Device->CreateRenderTargetView(SceneTexture, &SceneRTVDesc, &SceneRTV);
    if (FAILED(hr))
    {
        return hr;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC SceneSRVDesc = {};
    SceneSRVDesc.Format = SceneTextureDesc.Format;
    SceneSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SceneSRVDesc.Texture2D.MostDetailedMip = 0;
    SceneSRVDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(SceneTexture, &SceneSRVDesc, &SceneSRV);
    if (FAILED(hr))
    {
        return hr;
    }

    return hr;
}

HRESULT FRenderTarget::CreateDepthStencilResources(uint32 InWidth, uint32 InHeight)
{
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC DepthStencilTextureDesc = {};
    DepthStencilTextureDesc.Width = InWidth;
    DepthStencilTextureDesc.Height = InHeight;
    DepthStencilTextureDesc.MipLevels = 1;
    DepthStencilTextureDesc.ArraySize = 1;
    DepthStencilTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    DepthStencilTextureDesc.SampleDesc.Count = 1;
    DepthStencilTextureDesc.SampleDesc.Quality = 0;
    DepthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    DepthStencilTextureDesc.CPUAccessFlags = 0;
    DepthStencilTextureDesc.MiscFlags = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateTexture2D(&DepthStencilTextureDesc, nullptr, &DepthStencilTexture);
    if (FAILED(hr))
    {
        return hr;
    }
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(DepthStencilTexture,  &DepthStencilViewDesc,  &DepthStencilView);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC DepthStencilDesc = {};
    DepthStencilDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    DepthStencilDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    DepthStencilDesc.Texture2D.MostDetailedMip = 0;
    DepthStencilDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(DepthStencilTexture, &DepthStencilDesc, &DepthStencilSRV);
    if (FAILED(hr))
    {
        return hr;
    }

    return hr;
}


FViewport::FViewport()
    : FViewport(EViewScreenLocation::EVL_MAX)
{
}

FViewport::FViewport(EViewScreenLocation InViewLocation)
    : RenderTarget(new FRenderTarget())
    , ViewLocation(InViewLocation) 
{
}

FViewport::~FViewport()
{
    delete RenderTarget;
}

void FViewport::Initialize()
{
}

void FViewport::ResizeViewport(const DXGI_SWAP_CHAIN_DESC& SwapchainDesc)
{
    float width = (float)SwapchainDesc.BufferDesc.Width;
    float height = (float)SwapchainDesc.BufferDesc.Height;
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    switch (ViewLocation)
    {
    case EViewScreenLocation::EVL_TopLeft:
        GetD3DViewport().TopLeftX = 0.0f;
        GetD3DViewport().TopLeftY = 0.0f;
        GetD3DViewport().Width = halfWidth;
        GetD3DViewport().Height = halfHeight;
        break;
    case EViewScreenLocation::EVL_TopRight:
        GetD3DViewport().TopLeftX = halfWidth;
        GetD3DViewport().TopLeftY = 0.0f;
        GetD3DViewport().Width = halfWidth;
        GetD3DViewport().Height = halfHeight;
        break;
    case EViewScreenLocation::EVL_BottomLeft:
        GetD3DViewport().TopLeftX = 0.0f;
        GetD3DViewport().TopLeftY = halfHeight;
        GetD3DViewport().Width = halfWidth;
        GetD3DViewport().Height = halfHeight;
        break;
    case EViewScreenLocation::EVL_BottomRight:
        GetD3DViewport().TopLeftX = halfWidth;
        GetD3DViewport().TopLeftY = halfHeight;
        GetD3DViewport().Width = halfWidth;
        GetD3DViewport().Height = halfHeight;
        break;
    default:
        break;
    }
    GetD3DViewport().MinDepth = 0.0f;
    GetD3DViewport().MaxDepth = 1.0f;
}

void FViewport::ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right)
{
    switch (ViewLocation)
    {
    case EViewScreenLocation::EVL_TopLeft:
        GetD3DViewport().TopLeftX = Left.leftTopX;
        GetD3DViewport().TopLeftY = Top.leftTopY;
        GetD3DViewport().Width = Left.width;
        GetD3DViewport().Height = Top.height;
        break;
    case EViewScreenLocation::EVL_TopRight:
        GetD3DViewport().TopLeftX = Right.leftTopX;
        GetD3DViewport().TopLeftY = Top.leftTopY;
        GetD3DViewport().Width = Right.width;
        GetD3DViewport().Height = Top.height;
        break;
    case EViewScreenLocation::EVL_BottomLeft:
        GetD3DViewport().TopLeftX = Left.leftTopX;
        GetD3DViewport().TopLeftY = Bottom.leftTopY;
        GetD3DViewport().Width = Left.width;
        GetD3DViewport().Height = Bottom.height;
        break;
    case EViewScreenLocation::EVL_BottomRight:
        GetD3DViewport().TopLeftX = Right.leftTopX;
        GetD3DViewport().TopLeftY = Bottom.leftTopY;
        GetD3DViewport().Width = Right.width;
        GetD3DViewport().Height = Bottom.height;
        break;
    default:
        break;
    }
}

void FViewport::ResizeViewport(const FRect& NewRect)
{
    GetD3DViewport().TopLeftX = NewRect.leftTopX;
    GetD3DViewport().TopLeftY = NewRect.leftTopY;
    GetD3DViewport().Width = NewRect.width;
    GetD3DViewport().Height = NewRect.height;

    RenderTarget->Initialize(static_cast<uint32>(NewRect.width), static_cast<uint32>(NewRect.height));
}
