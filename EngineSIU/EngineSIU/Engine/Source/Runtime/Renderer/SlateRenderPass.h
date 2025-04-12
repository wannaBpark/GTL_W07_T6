
#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"

#include "Define.h"

struct FSlateTransform
{
    FVector2D Scale;
    FVector2D Offset;
};

class FSlateRenderPass : public IRenderPass
{
public:
    FSlateRenderPass();
    virtual ~FSlateRenderPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    
    virtual void PrepareRender() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

    void CreateShader();
    void ReleaseShader();

private:
    FDXDBufferManager* BufferManager;
    
    FGraphicsDevice* Graphics;
    
    FDXDShaderManager* ShaderManager;
};
