#include "DepthPrePass.h"

FDepthPrePass::FDepthPrePass()
{
}

FDepthPrePass::~FDepthPrePass()
{
}

void FDepthPrePass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;
    CreateShader();
}

void FDepthPrePass::PrepareRender()
{
}

void FDepthPrePass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FDepthPrePass::ClearRenderArr()
{
}
