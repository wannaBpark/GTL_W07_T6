#pragma once
#include "Define.h" 
#include <d3d11.h>

class FRenderTarget;

enum class EViewScreenLocation : uint8
{
    EVL_TopLeft,
    EVL_TopRight,
    EVL_BottomLeft,
    EVL_BottomRight,
    EVL_MAX,
};

class FRenderTarget
{
public:
    FRenderTarget();
    ~FRenderTarget();

    void Initialize(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 NewWidth, uint32 NewHeight);

    void Release();
    
    D3D11_VIEWPORT& GetD3DViewport() { return D3DViewport; }

    ID3D11Texture2D*& GetSceneTexture() { return SceneTexture; }
    ID3D11RenderTargetView*& GetSceneRTV() { return SceneRTV; }
    ID3D11ShaderResourceView*& GetSceneSRV() { return SceneSRV; }
    
    ID3D11Texture2D*& GetDepthStencilTexture() { return DepthStencilTexture; }
    ID3D11DepthStencilView*& GetDepthStencilView() { return DepthStencilView; }
    ID3D11ShaderResourceView*& GetDepthStencilSRV() { return DepthStencilSRV; }
    
    void SetViewport(const D3D11_VIEWPORT& InViewport) { D3DViewport = InViewport; }
    
    void SetSceneTexture(ID3D11Texture2D* InSceneTexture) { SceneTexture = InSceneTexture; }
    void SetSceneRTV(ID3D11RenderTargetView* InSceneRTV) { SceneRTV = InSceneRTV; }
    void SetSceneShaderResourceView(ID3D11ShaderResourceView* InSceneSRV) { SceneSRV = InSceneSRV; }
    
    void SetDepthStencilTexture(ID3D11Texture2D* InDepthStencilTexture) { DepthStencilTexture = InDepthStencilTexture; }
    void SetDepthStencilView(ID3D11DepthStencilView* InDepthStencilView) { DepthStencilView = InDepthStencilView; }
    void SetDepthStencilSRV(ID3D11ShaderResourceView* InDepthStencilSRV) { DepthStencilSRV = InDepthStencilSRV; }

    void ClearRenderTarget(ID3D11DeviceContext* DeviceContext);
    
private:
    // DirectX
    D3D11_VIEWPORT D3DViewport = {};

    ID3D11Texture2D* SceneTexture = nullptr;
    ID3D11RenderTargetView* SceneRTV = nullptr;
    ID3D11ShaderResourceView* SceneSRV = nullptr;

    ID3D11Texture2D* DepthStencilTexture = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;
    ID3D11ShaderResourceView* DepthStencilSRV = nullptr;

    HRESULT CreateSceneResources(uint32 InWidth, uint32 InHeight);
    HRESULT CreateDepthStencilResources(uint32 InWidth, uint32 InHeight);

    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
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

    FRenderTarget* GetRenderTarget() const { return RenderTarget; }

private:
    FRenderTarget* RenderTarget;

    EViewScreenLocation ViewLocation;   // 뷰포트 위치
};
