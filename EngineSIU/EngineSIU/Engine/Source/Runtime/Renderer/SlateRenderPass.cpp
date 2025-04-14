#include "SlateRenderPass.h"

FSlateRenderPass::FSlateRenderPass()
{
}

FSlateRenderPass::~FSlateRenderPass()
{
}

void FSlateRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
}

void FSlateRenderPass::PrepareRender()
{
}

void FSlateRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
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
