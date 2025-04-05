#pragma once

#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"
#include "Define.h"

class FDXDShaderManager;
class UWorld;
class FEditorViewportClient;
class ULightComponentBase;

class FLightRenderPass : public IRenderPass
{
public:
    FLightRenderPass();
    ~FLightRenderPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRender() override;
    virtual void Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;
    void UpdateLightBuffer() const;

private:
    TArray<ULightComponentBase*> LightObjs;

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
};
