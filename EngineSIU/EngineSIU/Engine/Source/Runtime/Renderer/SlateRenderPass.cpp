#include "SlateRenderPass.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

FSlateRenderPass::FSlateRenderPass()
{
}

FSlateRenderPass::~FSlateRenderPass()
{
}

void FSlateRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FSlateRenderPass::PrepareRender()
{
}

void FSlateRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    /*
    float LeftNDC   = (2.0f * LeftOffset) / ScreenWidth - 1.0f;
    float RightNDC  = (2.0f * (LeftOffset + Width)) / ScreenWidth - 1.0f;
    float TopNDC    = 1.0f - (2.0f * TopOffset) / ScreenHeight;
    float BottomNDC = 1.0f - (2.0f * (TopOffset + Height)) / ScreenHeight;

    float ScaleX = (RightNDC - LeftNDC) * 0.5f;
    float ScaleY = (TopNDC - BottomNDC) * 0.5f;

    float OffsetX = (LeftNDC + RightNDC) * 0.5f;
    float OffsetY = (TopNDC + BottomNDC) * 0.5f;
    */
}

void FSlateRenderPass::ClearRenderArr()
{
}

void FSlateRenderPass::CreateShader()
{

}

void FSlateRenderPass::ReleaseShader()
{
}
