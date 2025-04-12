#include "UnrealClient.h"

#include "EngineLoop.h"

FRenderTargetRHI::FRenderTargetRHI()
{
}

FRenderTargetRHI::~FRenderTargetRHI()
{
    Release();
}

void FRenderTargetRHI::Initialize(uint32 InWidth, uint32 InHeight)
{
    D3DViewport.Height = static_cast<float>(InHeight);
    D3DViewport.Width = static_cast<float>(InWidth);
    D3DViewport.MaxDepth = 1.0f;
    D3DViewport.MinDepth = 0.0f;

    HRESULT hr = S_OK;
    hr = CreateDepthStencilResources(InWidth, InHeight);
    if (FAILED(hr))
    {
        return;
    }

    // Essential resources
    hr = CreateResource(EResourceType::ERT_Final, InWidth, InHeight);
    if (FAILED(hr))
    {
        return;
    }

    hr = CreateResource(EResourceType::ERT_Scene, InWidth, InHeight);
    if (FAILED(hr))
    {
        return;
    }
}

void FRenderTargetRHI::Resize(uint32 NewWidth, uint32 NewHeight)
{
    D3DViewport.Width = static_cast<float>(NewWidth);
    D3DViewport.Height = static_cast<float>(NewHeight);
    
    Release();
    Initialize(NewWidth, NewHeight);
}

void FRenderTargetRHI::Release()
{
    ReleaseDepthStencilResources();
    ReleaseResources();
}

FViewportResources* FRenderTargetRHI::GetResource(EResourceType Type)
{
    if (Resources.Contains(Type))
    {
        return Resources.Find(Type);
    }
    return nullptr;
}

void FRenderTargetRHI::ClearRenderTarget(ID3D11DeviceContext* DeviceContext)
{
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    DeviceContext->ClearRenderTargetView(GetResource(EResourceType::ERT_Scene)->RTV, ClearColor);
}

HRESULT FRenderTargetRHI::CreateDepthStencilResources(uint32 InWidth, uint32 InHeight)
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

HRESULT FRenderTargetRHI::CreateResource(EResourceType Type, uint32 InWidth, uint32 InHeight)
{
    FViewportResources NewResource;
    
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = InWidth;
    TextureDesc.Height = InHeight;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;
    NewResource.Texture2D = FEngineLoop::GraphicDevice.CreateTexture2D(TextureDesc, nullptr);

    D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
    RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: srgb 옵션 고려해보기
    RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = FEngineLoop::GraphicDevice.Device->CreateRenderTargetView(NewResource.Texture2D, &RTVDesc, &NewResource.RTV);
    if (FAILED(hr))
    {
        return hr;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = TextureDesc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    SRVDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D, &SRVDesc, &NewResource.SRV);
    if (FAILED(hr))
    {
        return hr;
    }

    Resources.Add(Type, NewResource);

    return hr;
}

void FRenderTargetRHI::ReleaseDepthStencilResources()
{
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
    if (DepthStencilTexture)
    {
        DepthStencilTexture->Release();
        DepthStencilTexture = nullptr;
    }
}

void FRenderTargetRHI::ReleaseResources()
{
    for (auto& [Type, Resource] : Resources)
    {
        Resource.Release();
    }
}


FViewport::FViewport()
    : FViewport(EViewScreenLocation::EVL_MAX)
{
}

FViewport::FViewport(EViewScreenLocation InViewLocation)
    : RenderTarget(new FRenderTargetRHI())
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
        GetD3DViewport().TopLeftX = Left.LeftTopX;
        GetD3DViewport().TopLeftY = Top.LeftTopY;
        GetD3DViewport().Width = Left.Width;
        GetD3DViewport().Height = Top.Height;
        break;
    case EViewScreenLocation::EVL_TopRight:
        GetD3DViewport().TopLeftX = Right.LeftTopX;
        GetD3DViewport().TopLeftY = Top.LeftTopY;
        GetD3DViewport().Width = Right.Width;
        GetD3DViewport().Height = Top.Height;
        break;
    case EViewScreenLocation::EVL_BottomLeft:
        GetD3DViewport().TopLeftX = Left.LeftTopX;
        GetD3DViewport().TopLeftY = Bottom.LeftTopY;
        GetD3DViewport().Width = Left.Width;
        GetD3DViewport().Height = Bottom.Height;
        break;
    case EViewScreenLocation::EVL_BottomRight:
        GetD3DViewport().TopLeftX = Right.LeftTopX;
        GetD3DViewport().TopLeftY = Bottom.LeftTopY;
        GetD3DViewport().Width = Right.Width;
        GetD3DViewport().Height = Bottom.Height;
        break;
    default:
        break;
    }
}

void FViewport::ResizeViewport(const FRect& NewRect)
{
    GetD3DViewport().TopLeftX = NewRect.LeftTopX;
    GetD3DViewport().TopLeftY = NewRect.LeftTopY;
    GetD3DViewport().Width = NewRect.Width;
    GetD3DViewport().Height = NewRect.Height;

    RenderTarget->Resize(static_cast<uint32>(NewRect.Width), static_cast<uint32>(NewRect.Height));
}
