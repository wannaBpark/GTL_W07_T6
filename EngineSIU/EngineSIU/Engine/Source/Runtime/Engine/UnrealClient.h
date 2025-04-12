#pragma once
#include "Define.h" 
#include <d3d11.h>

#include "Container/Map.h"

class FRenderTargetRHI;

enum class EViewScreenLocation : uint8
{
    EVL_TopLeft,
    EVL_TopRight,
    EVL_BottomLeft,
    EVL_BottomRight,
    EVL_MAX,
};

enum class EResourceType : uint8
{
    ERT_Final,
    ERT_Scene,
    ERT_Depth,
    ERT_WorldNormal,
    ERT_PP_Fog,
    ERT_MAX,
};

struct FViewportResources
{
    ID3D11Texture2D* Texture2D;
    ID3D11RenderTargetView* RTV;
    ID3D11ShaderResourceView* SRV;

    void Release()
    {
        if (SRV)
        {
            SRV->Release();
            SRV = nullptr;
        }
        if (RTV)
        {
            RTV->Release();
            RTV = nullptr;
        }
        if (Texture2D)
        {
            Texture2D->Release();
            Texture2D = nullptr;
        }
    }
};

class FRenderTargetRHI
{
public:
    FRenderTargetRHI();
    ~FRenderTargetRHI();

    void Initialize(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 NewWidth, uint32 NewHeight);

    void Release();
    
    D3D11_VIEWPORT& GetD3DViewport() { return D3DViewport; }
    
    ID3D11Texture2D*& GetDepthStencilTexture() { return DepthStencilTexture; }
    ID3D11DepthStencilView*& GetDepthStencilView() { return DepthStencilView; }
    ID3D11ShaderResourceView*& GetDepthStencilSRV() { return DepthStencilSRV; }

    FViewportResources* GetResource(EResourceType Type);
    
    void ClearRenderTarget(ID3D11DeviceContext* DeviceContext);
    
private:
    // DirectX
    D3D11_VIEWPORT D3DViewport = {};

    ID3D11Texture2D* DepthStencilTexture = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;
    ID3D11ShaderResourceView* DepthStencilSRV = nullptr;

    TMap<EResourceType, FViewportResources> Resources;

    HRESULT CreateDepthStencilResources(uint32 InWidth, uint32 InHeight);
    HRESULT CreateResource(EResourceType Type, uint32 InWidth, uint32 InHeight);

    void ReleaseDepthStencilResources();
    void ReleaseResources();

    float ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
};


class FViewport
{
public:
    FViewport();
    FViewport(EViewScreenLocation InViewLocation);
    ~FViewport();

    void Initialize();
    void ResizeViewport(const DXGI_SWAP_CHAIN_DESC& SwapchainDesc);
    void ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right);
    void ResizeViewport(const FRect& NewRect);

    D3D11_VIEWPORT& GetD3DViewport() const { return RenderTarget->GetD3DViewport(); }

    EViewScreenLocation GetViewLocation() const { return ViewLocation; }

    FRenderTargetRHI* GetRenderTarget() const { return RenderTarget; }

private:
    FRenderTargetRHI* RenderTarget;

    EViewScreenLocation ViewLocation;   // 뷰포트 위치
};
