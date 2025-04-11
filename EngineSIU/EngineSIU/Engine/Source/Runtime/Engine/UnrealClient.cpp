#include "UnrealClient.h"

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
    case EViewScreenLocation::TopLeft:
        D3DViewport.TopLeftX = 0.0f;
        D3DViewport.TopLeftY = 0.0f;
        D3DViewport.Width = halfWidth;
        D3DViewport.Height = halfHeight;
        break;
    case EViewScreenLocation::TopRight:
        D3DViewport.TopLeftX = halfWidth;
        D3DViewport.TopLeftY = 0.0f;
        D3DViewport.Width = halfWidth;
        D3DViewport.Height = halfHeight;
        break;
    case EViewScreenLocation::BottomLeft:
        D3DViewport.TopLeftX = 0.0f;
        D3DViewport.TopLeftY = halfHeight;
        D3DViewport.Width = halfWidth;
        D3DViewport.Height = halfHeight;
        break;
    case EViewScreenLocation::BottomRight:
        D3DViewport.TopLeftX = halfWidth;
        D3DViewport.TopLeftY = halfHeight;
        D3DViewport.Width = halfWidth;
        D3DViewport.Height = halfHeight;
        break;
    default:
        break;
    }
    D3DViewport.MinDepth = 0.0f;
    D3DViewport.MaxDepth = 1.0f;
}

void FViewport::ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right)
{
    switch (ViewLocation)
    {
    case EViewScreenLocation::TopLeft:
        D3DViewport.TopLeftX = Left.leftTopX;
        D3DViewport.TopLeftY = Top.leftTopY;
        D3DViewport.Width = Left.width;
        D3DViewport.Height = Top.height;
        break;
    case EViewScreenLocation::TopRight:
        D3DViewport.TopLeftX = Right.leftTopX;
        D3DViewport.TopLeftY = Top.leftTopY;
        D3DViewport.Width = Right.width;
        D3DViewport.Height = Top.height;
        break;
    case EViewScreenLocation::BottomLeft:
        D3DViewport.TopLeftX = Left.leftTopX;
        D3DViewport.TopLeftY = Bottom.leftTopY;
        D3DViewport.Width = Left.width;
        D3DViewport.Height = Bottom.height;
        break;
    case EViewScreenLocation::BottomRight:
        D3DViewport.TopLeftX = Right.leftTopX;
        D3DViewport.TopLeftY = Bottom.leftTopY;
        D3DViewport.Width = Right.width;
        D3DViewport.Height = Bottom.height;
        break;
    default:
        break;
    }
}

void FViewport::ResizeViewport(const FRect& newRect)
{
    D3DViewport.TopLeftX = newRect.leftTopX;
    D3DViewport.TopLeftY = newRect.leftTopY;
    D3DViewport.Width = newRect.width;
    D3DViewport.Height = newRect.height;
}

