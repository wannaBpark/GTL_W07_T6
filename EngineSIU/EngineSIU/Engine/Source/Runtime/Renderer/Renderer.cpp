
#include "Renderer.h"

#include <array>

#include "World/World.h"
#include "Engine/EditorEngine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "RendererHelpers.h"
#include "StaticMeshRenderPass.h"
#include "BillboardRenderPass.h"
#include "GizmoRenderPass.h"
#include "UpdateLightBufferPass.h"
#include "LineRenderPass.h"
#include "DepthBufferDebugPass.h"
#include "FogRenderPass.h"
#include "SlateRenderPass.h"
#include <UObject/UObjectIterator.h>
#include <UObject/Casts.h>

#include "CompositingPass.h"
#include "SceneRenderPass.h"
#include "SlateRenderPass.h"
#include "UnrealClient.h"
#include "WorldNormalDebugPass.h"
#include "GameFrameWork/Actor.h"

#include "PropertyEditor/ShowFlags.h"

//------------------------------------------------------------------------------
// 초기화 및 해제 관련 함수
//------------------------------------------------------------------------------
void FRenderer::Initialize(FGraphicsDevice* InGraphics, FDXDBufferManager* InBufferManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;

    ShaderManager = new FDXDShaderManager(Graphics->Device);
    
    StaticMeshRenderPass = new FStaticMeshRenderPass();
    BillboardRenderPass = new FBillboardRenderPass();
    GizmoRenderPass = new FGizmoRenderPass();
    UpdateLightBufferPass = new FUpdateLightBufferPass();
    LineRenderPass = new FLineRenderPass();
    FogRenderPass = new FFogRenderPass();
    CompositingPass = new FCompositingPass();
    SlateRenderPass = new FSlateRenderPass();

    StaticMeshRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    BillboardRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    GizmoRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    UpdateLightBufferPass->Initialize(BufferManager, Graphics, ShaderManager);
    LineRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    FogRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    
    CompositingPass->Initialize(BufferManager, Graphics, ShaderManager);
    
    SlateRenderPass->Initialize(BufferManager, Graphics, ShaderManager);

    CreateConstantBuffers();
}

void FRenderer::Release()
{
    delete ShaderManager;

    // TODO: 생성한 렌더 패스 객체 모두 해제
}

//------------------------------------------------------------------------------
// 사용하는 모든 상수 버퍼 생성
//------------------------------------------------------------------------------
void FRenderer::CreateConstantBuffers()
{
    UINT perObjectBufferSize = sizeof(FPerObjectConstantBuffer);
    BufferManager->CreateBufferGeneric<FPerObjectConstantBuffer>("FPerObjectConstantBuffer", nullptr, perObjectBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT cameraConstantBufferSize = sizeof(FCameraConstantBuffer);
    BufferManager->CreateBufferGeneric<FCameraConstantBuffer>("FCameraConstantBuffer", nullptr, cameraConstantBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT subUVBufferSize = sizeof(FSubUVConstant);
    BufferManager->CreateBufferGeneric<FSubUVConstant>("FSubUVConstant", nullptr, subUVBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT materialBufferSize = sizeof(FMaterialConstants);
    BufferManager->CreateBufferGeneric<FMaterialConstants>("FMaterialConstants", nullptr, materialBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT subMeshBufferSize = sizeof(FSubMeshConstants);
    BufferManager->CreateBufferGeneric<FSubMeshConstants>("FSubMeshConstants", nullptr, subMeshBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT textureBufferSize = sizeof(FTextureConstants);
    BufferManager->CreateBufferGeneric<FTextureConstants>("FTextureConstants", nullptr, textureBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT lightingBufferSize = sizeof(FLightBuffer);
    BufferManager->CreateBufferGeneric<FLightBuffer>("FLightBuffer", nullptr, lightingBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT litUnlitBufferSize = sizeof(FLitUnlitConstants);
    BufferManager->CreateBufferGeneric<FLitUnlitConstants>("FLitUnlitConstants", nullptr, litUnlitBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT ViewModeBufferSize = sizeof(FViewModeConstants);
    BufferManager->CreateBufferGeneric<FViewModeConstants>("FViewModeConstants", nullptr, ViewModeBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT ScreenConstantsBufferSize = sizeof(FScreenConstants);
    BufferManager->CreateBufferGeneric<FScreenConstants>("FScreenConstants", nullptr, ScreenConstantsBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT FogConstantBufferSize = sizeof(FFogConstants);
    BufferManager->CreateBufferGeneric<FFogConstants>("FFogConstants", nullptr, FogConstantBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void FRenderer::ReleaseConstantBuffer()
{
    BufferManager->ReleaseConstantBuffer();
}

void FRenderer::PrepareRender()
{
    StaticMeshRenderPass->PrepareRender();
    GizmoRenderPass->PrepareRender();
    BillboardRenderPass->PrepareRender();
    UpdateLightBufferPass->PrepareRender();
    FogRenderPass->PrepareRender();
}

void FRenderer::ClearRenderArr()
{
    StaticMeshRenderPass->ClearRenderArr();
    BillboardRenderPass->ClearRenderArr();
    GizmoRenderPass->ClearRenderArr();
    UpdateLightBufferPass->ClearRenderArr();
    FogRenderPass->ClearRenderArr();
}

void FRenderer::SetRenderResource(EResourceType Type, FRenderTargetRHI* RenderTargetRHI)
{
    FViewportResources* ResourceRHI = RenderTargetRHI->GetResource(Type);
    if (!ResourceRHI)
    {
        return;
    }

    Graphics->DeviceContext->OMSetRenderTargets(1, &ResourceRHI->RTV, RenderTargetRHI->GetDepthStencilView());
    Graphics->DeviceContext->ClearDepthStencilView(RenderTargetRHI->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    Graphics->DeviceContext->ClearRenderTargetView(ResourceRHI->RTV, RenderTargetRHI->GetClearColor(Type).data());
}

void FRenderer::Render(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    // Setup
    FRenderTargetRHI* RenderTargetRHI = ActiveViewport->GetRenderTargetRHI();
    if (!RenderTargetRHI)
    {
        return;
    }

    Graphics->DeviceContext->RSSetViewports(1, &RenderTargetRHI->GetD3DViewport());

    const uint64 ShowFlag = ActiveViewport->GetShowFlag();
    
    // Render Scene
    SetRenderResource(EResourceType::ERT_Scene, RenderTargetRHI);

    if (ShowFlag & EEngineShowFlags::SF_Primitives)
    {
        UpdateLightBufferPass->Render(ActiveViewport);
        StaticMeshRenderPass->Render(ActiveViewport);
    }

    // Render Postprocess
    if (ActiveViewport->GetViewMode() == VMI_Lit)
    {
        if (ShowFlag & EEngineShowFlags::SF_Fog)
        {
            // TODO: 여기에서는 씬 렌더가 적용된 뎁스 스텐실 뷰를 바인딩 해제해서 SRV로 전달해야 함
        }
    }
    
    // Render Billboard
    if (ShowFlag & EEngineShowFlags::SF_BillboardText)
    {
        BillboardRenderPass->Render(ActiveViewport);
    }

    // Render for Editor
    if (GEngine->ActiveWorld->WorldType != EWorldType::PIE && false)  // TODO: false 제거
    {
        LineRenderPass->Render(ActiveViewport); // TODO: 여기에서는 기존 뎁스를 그대로 사용하고
        GizmoRenderPass->Render(ActiveViewport); // TODO: 여기에서는 기존 뎁스를 SRV로 전달해서 샘플 후 비교해야 함
    }

    // Compositing
    CompositingPass->Render(ActiveViewport);

    ClearRenderArr();
}
