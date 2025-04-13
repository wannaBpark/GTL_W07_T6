#include "UnrealClient.h"

#include "EngineLoop.h"
#include <array>

FRenderTargetRHI::FRenderTargetRHI()
{
    ClearColors.Add(EResourceType::ERT_Compositing, { 0.f, 0.f, 0.f, 1.f });
    ClearColors.Add(EResourceType::ERT_Scene,  { 0.025f, 0.025f, 0.025f, 1.0f });
    ClearColors.Add(EResourceType::ERT_PP_Fog, { 0.f, 0.f, 0.f, 0.f });
    ClearColors.Add(EResourceType::ERT_Editor, { 0.f, 0.f, 0.f, 0.f });
    ClearColors.Add(EResourceType::ERT_Overlay, { 0.f, 0.f, 0.f, 0.f });
}

FRenderTargetRHI::~FRenderTargetRHI()
{
    Release();

    ClearColors.Empty();
}

void FRenderTargetRHI::Initialize(uint32 InWidth, uint32 InHeight)
{
    D3DViewport.TopLeftX = 0.f;
    D3DViewport.TopLeftY = 0.f;
    D3DViewport.Height = static_cast<float>(InHeight);
    D3DViewport.Width = static_cast<float>(InWidth);
    D3DViewport.MaxDepth = 1.0f;
    D3DViewport.MinDepth = 0.0f;

    HRESULT hr = S_OK;
    hr = CreateDepthStencilResources();
    if (FAILED(hr))
    {
        return;
    }

    // Essential resources
    hr = CreateResource(EResourceType::ERT_Compositing);
    if (FAILED(hr))
    {
        return;
    }

    hr = CreateResource(EResourceType::ERT_Scene);
    if (FAILED(hr))
    {
        return;
    }
}

void FRenderTargetRHI::Resize(uint32 NewWidth, uint32 NewHeight)
{
    Release();
    Initialize(NewWidth, NewHeight);
}

void FRenderTargetRHI::Release()
{
    ReleaseDepthStencilResources();
    ReleaseResources();
}

HRESULT FRenderTargetRHI::CreateResource(EResourceType Type)
{
    if (HasResource(Type))
    {
        ReleaseResource(Type);
    }
    
    FViewportResources NewResource;
    
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = static_cast<uint32>(D3DViewport.Width);
    TextureDesc.Height = static_cast<uint32>(D3DViewport.Height);
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

bool FRenderTargetRHI::HasResource(EResourceType Type) const
{
    return Resources.Contains(Type);
}

FViewportResources* FRenderTargetRHI::GetResource(EResourceType Type)
{
    if (!Resources.Contains(Type))
    {
        if (FAILED(CreateResource(Type)))
        {
            return nullptr;
        }
    }
    return Resources.Find(Type);
}

void FRenderTargetRHI::ClearRenderTargets(ID3D11DeviceContext* DeviceContext)
{
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    for (auto& [Type, Resource] : Resources)
    {
        DeviceContext->ClearRenderTargetView(Resource.RTV, ClearColors[Type].data());
    }
}

std::array<float, 4> FRenderTargetRHI::GetClearColor(EResourceType Type) const
{
    return ClearColors[Type];
}

HRESULT FRenderTargetRHI::CreateDepthStencilResources()
{
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC DepthStencilTextureDesc = {};
    DepthStencilTextureDesc.Width = static_cast<uint32>(D3DViewport.Width);
    DepthStencilTextureDesc.Height = static_cast<uint32>(D3DViewport.Height);
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
    hr = FEngineLoop::GraphicDevice.Device->CreateTexture2D(&DepthStencilTextureDesc, nullptr, &GizmoDepthStencilTexture);
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
    hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(GizmoDepthStencilTexture,  &DepthStencilViewDesc,  &GizmoDepthStencilView);
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

void FRenderTargetRHI::ReleaseResource(EResourceType Type)
{
    if (HasResource(Type))
    {
        Resources[Type].Release();
    }
}


FViewport::FViewport()
    : FViewport(EViewScreenLocation::EVL_MAX)
{
}

FViewport::FViewport(EViewScreenLocation InViewLocation)
    : RenderTargetRHI(new FRenderTargetRHI())
    , ViewLocation(InViewLocation) 
{
}

FViewport::~FViewport()
{
    delete RenderTargetRHI;
}

void FViewport::Initialize(const FRect& InRect)
{
    Rect = InRect;
    const uint32 Width = static_cast<uint32>(Rect.Width);
    const uint32 Height = static_cast<uint32>(Rect.Height);

    RenderTargetRHI->Initialize(Width, Height);
}

void FViewport::ResizeViewport(const FRect& InRect)
{
    Rect = InRect;
    const uint32 Width = static_cast<uint32>(Rect.Width);
    const uint32 Height = static_cast<uint32>(Rect.Height);

    RenderTargetRHI->Resize(Width, Height);
}

void FViewport::ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right)
{
    switch (ViewLocation)
    {
    case EViewScreenLocation::EVL_TopLeft:
        Rect.TopLeftX = Left.TopLeftX;
        Rect.TopLeftY = Top.TopLeftY;
        Rect.Width = Left.Width;
        Rect.Height = Top.Height;
        break;
    case EViewScreenLocation::EVL_TopRight:
        Rect.TopLeftX = Right.TopLeftX;
        Rect.TopLeftY = Top.TopLeftY;
        Rect.Width = Right.Width;
        Rect.Height = Top.Height;
        break;
    case EViewScreenLocation::EVL_BottomLeft:
        Rect.TopLeftX = Left.TopLeftX;
        Rect.TopLeftY = Bottom.TopLeftY;
        Rect.Width = Left.Width;
        Rect.Height = Bottom.Height;
        break;
    case EViewScreenLocation::EVL_BottomRight:
        Rect.TopLeftX = Right.TopLeftX;
        Rect.TopLeftY = Bottom.TopLeftY;
        Rect.Width = Right.Width;
        Rect.Height = Bottom.Height;
        break;
    default:
        return;
    }
    
    const uint32 Width = static_cast<uint32>(Rect.Width);
    const uint32 Height = static_cast<uint32>(Rect.Height);
    RenderTargetRHI->Resize(Width, Height);
}
