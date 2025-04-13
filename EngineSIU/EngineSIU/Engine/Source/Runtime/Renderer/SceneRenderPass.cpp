#include "SceneRenderPass.h"

#include <array>

#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"

#include "UpdateLightBufferPass.h"
#include "StaticMeshRenderPass.h"
#include "BillboardRenderPass.h"

FSceneRenderPass::FSceneRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
    UpdateLightBufferPass = new FUpdateLightBufferPass();
    StaticMeshRenderPass = new FStaticMeshRenderPass();
    BillboardRenderPass = new FBillboardRenderPass();
}

FSceneRenderPass::~FSceneRenderPass()
{
    delete UpdateLightBufferPass;
    delete StaticMeshRenderPass;
    delete BillboardRenderPass;
}

void FSceneRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;

    UpdateLightBufferPass->Initialize(InBufferManager, InGraphics, InShaderManage);
    StaticMeshRenderPass->Initialize(InBufferManager, InGraphics, InShaderManage);
    BillboardRenderPass->Initialize(InBufferManager, InGraphics, InShaderManage);
}

void FSceneRenderPass::PrepareRender()
{
}

void FSceneRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // TODO: 뷰포트의 Scene 리소스를 통해 렌더 타겟 뷰와 뎁스 스텐실 뷰를 설정하여 스태틱 메시, 빌보드, 텍스트를 렌더

    // Setup
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    if (!RenderTargetRHI)
    {
        return;
    }

    const EResourceType ResourceType = EResourceType::ERT_Scene;
    FViewportResources* ResourceRHI = Viewport->GetRenderTargetRHI()->GetResource(ResourceType);
    if (!ResourceRHI)
    {
        return;
    }

    Graphics->DeviceContext->OMSetRenderTargets(1, &ResourceRHI->RTV, RenderTargetRHI->GetDepthStencilView());
    Graphics->DeviceContext->ClearDepthStencilView(RenderTargetRHI->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    Graphics->DeviceContext->ClearRenderTargetView(ResourceRHI->RTV, RenderTargetRHI->GetClearColor(ResourceType).data());

    Graphics->ChangeRasterizer(Viewport->GetViewMode());

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Render
    UpdateLightBufferPass->Render(Viewport);

    StaticMeshRenderPass->ChangeViewMode(Viewport->GetViewMode());
    StaticMeshRenderPass->Render(Viewport);
    
    BillboardRenderPass->Render(Viewport);

    // Finish
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(100, 1, &ResourceRHI->SRV);
}

void FSceneRenderPass::ClearRenderArr()
{
}
