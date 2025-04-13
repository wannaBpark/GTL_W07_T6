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
    ERT_Compositing,
    ERT_Scene,
    ERT_PP_Fog,
    ERT_Editor,
    ERT_Overlay,
    ERT_MAX,
};

struct FViewportResources
{
    ~FViewportResources()
    {
        Release();
    }
    
    ID3D11Texture2D* Texture2D = nullptr;
    ID3D11RenderTargetView* RTV = nullptr;
    ID3D11ShaderResourceView* SRV = nullptr;

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

    HRESULT CreateResource(EResourceType Type);
    
    D3D11_VIEWPORT& GetD3DViewport() { return D3DViewport; }
    
    ID3D11Texture2D*& GetDepthStencilTexture() { return DepthStencilTexture; }
    ID3D11DepthStencilView*& GetDepthStencilView() { return DepthStencilView; }
    ID3D11ShaderResourceView*& GetDepthStencilSRV() { return DepthStencilSRV; }

    ID3D11Texture2D*& GetGizmoDepthStencilTexture() { return GizmoDepthStencilTexture; }
    ID3D11DepthStencilView*& GetGizmoDepthStencilView() { return GizmoDepthStencilView; }

    FViewportResources* GetResource(EResourceType Type);

    bool HasResource(EResourceType Type) const;
    
    void ClearRenderTargets(ID3D11DeviceContext* DeviceContext);

    std::array<float, 4> GetClearColor(EResourceType Type) const;
    
private:
    // DirectX
    D3D11_VIEWPORT D3DViewport = {};

    ID3D11Texture2D* DepthStencilTexture = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;
    ID3D11ShaderResourceView* DepthStencilSRV = nullptr;

    ID3D11Texture2D* GizmoDepthStencilTexture = nullptr;
    ID3D11DepthStencilView* GizmoDepthStencilView = nullptr;

    TMap<EResourceType, FViewportResources> Resources;

    HRESULT CreateDepthStencilResources();

    void ReleaseDepthStencilResources();
    void ReleaseResources();
    void ReleaseResource(EResourceType Type);

    TMap<EResourceType, std::array<float, 4>> ClearColors;
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

    D3D11_VIEWPORT& GetD3DViewport() const { return RenderTargetRHI->GetD3DViewport(); }

    EViewScreenLocation GetViewLocation() const { return ViewLocation; }

    FRenderTargetRHI* GetRenderTargetRHI() const { return RenderTargetRHI; }

private:
    FRenderTargetRHI* RenderTargetRHI;

    EViewScreenLocation ViewLocation;   // 뷰포트 위치
};
