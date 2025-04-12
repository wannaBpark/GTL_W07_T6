#pragma once
#include "IRenderPass.h"
#include "Define.h"

class FDXDShaderManager;
class FGraphicsDevice;
class FDXDBufferManager;
class UPointLightComponent;
class UDirectionalLightComponent;
class USpotLightComponent;
class UAmbientLightComponent;

class FEditorRenderPass : public IRenderPass
{
public:
    FEditorRenderPass();
    ~FEditorRenderPass();
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

    void CreateBuffers();

private:
    TArray<UPointLightComponent*> PointLights;
    TArray<USpotLightComponent*> SpotLights;
    TArray<UDirectionalLightComponent*> DirectionalLights;
    TArray<UAmbientLightComponent*> AmbientLights;
};

