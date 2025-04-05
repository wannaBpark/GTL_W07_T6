#pragma once

#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"

class UGizmoBaseComponent;

class FDXDBufferManager;

class FGraphicsDevice;

class FDXDShaderManager;

class UWorld;

class FEditorViewportClient;

class FGizmoRenderPass : public IRenderPass
{
public:
    FGizmoRenderPass();
    virtual ~FGizmoRenderPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;

    virtual void PrepareRender() override;

    virtual void Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

    // Gizmo 한 개 렌더링 함수
    void RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& Viewport, const UWorld* World);

private:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    // 수집된 Gizmo 객체 배열
    TArray<UGizmoBaseComponent*> GizmoObjs;
};
