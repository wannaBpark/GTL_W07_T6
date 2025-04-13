#pragma once
#include "IRenderPass.h"

class FUpdateLightBufferPass;
class FStaticMeshRenderPass;
class FBillboardRenderPass;

class FSceneRenderPass : public IRenderPass
{
public:
    FSceneRenderPass();
    
    virtual ~FSceneRenderPass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    
    virtual void PrepareRender() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

private:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    FUpdateLightBufferPass* UpdateLightBufferPass;
    FStaticMeshRenderPass* StaticMeshRenderPass;
    FBillboardRenderPass* BillboardRenderPass;
};
