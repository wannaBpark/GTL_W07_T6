#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"

#include "Define.h"
#include <d3d11.h>
class FDXDShaderManager;
class FGraphicsDevice;
class FDXDBufferManager;

class FDepthPrePass : public IRenderPass
{
    friend class FRenderer; // 렌더러에서 접근 가능
    friend class DepthBufferDebugPass; // DepthBufferDebugPass에서 접근 가능
public:
    FDepthPrePass();
    ~FDepthPrePass();
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

    void CreateDepthStencilState();

    void Release();

    void ResizeDepthStencil();

private:
    FDXDBufferManager* BufferManager;

    FGraphicsDevice* Graphics;

    FDXDShaderManager* ShaderManager;
private:

    ID3D11DepthStencilState* DepthStencilState_OnlyWrite = nullptr;
    ID3D11Texture2D* DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;
    ID3D11ShaderResourceView* DepthSRV = nullptr;
};

