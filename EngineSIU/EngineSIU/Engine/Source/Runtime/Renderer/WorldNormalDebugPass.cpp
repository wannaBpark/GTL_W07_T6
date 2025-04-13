#include "WorldNormalDebugPass.h"

FWorldNormalDebugPass::FWorldNormalDebugPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FWorldNormalDebugPass::~FWorldNormalDebugPass()
{
}

void FWorldNormalDebugPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;
}

void FWorldNormalDebugPass::PrepareRender()
{
}

void FWorldNormalDebugPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FWorldNormalDebugPass::ClearRenderArr()
{
}
