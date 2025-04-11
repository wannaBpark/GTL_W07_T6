#pragma once
#include "Define.h" 
#include <d3d11.h>


enum class EViewScreenLocation : uint8
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

class FViewport
{
public:
    FViewport() = default;
    FViewport(EViewScreenLocation InViewLocation) : ViewLocation(InViewLocation) {}

    void Initialize();
    void ResizeViewport(const DXGI_SWAP_CHAIN_DESC& SwapchainDesc);
    void ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right);
    void ResizeViewport(const FRect& newRect);

private:
    D3D11_VIEWPORT D3DViewport;            // 뷰포트 정보
    EViewScreenLocation ViewLocation;   // 뷰포트 위치
    
public:
    D3D11_VIEWPORT& GetViewport() { return D3DViewport; }
    void SetViewport(const D3D11_VIEWPORT& _viewport) { D3DViewport = _viewport; }
};
