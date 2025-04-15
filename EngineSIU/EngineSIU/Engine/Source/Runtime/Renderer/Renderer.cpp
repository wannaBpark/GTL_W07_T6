
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
#include "FogRenderPass.h"
#include "SlateRenderPass.h"
#include <UObject/UObjectIterator.h>
#include <UObject/Casts.h>

#include "CompositingPass.h"
#include "SlateRenderPass.h"
#include "UnrealClient.h"
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

    CreateConstantBuffers();
    CreateCommonShader();
    
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
    UINT ObjectBufferSize = sizeof(FObjectConstantBuffer);
    BufferManager->CreateBufferGeneric<FObjectConstantBuffer>("FObjectConstantBuffer", nullptr, ObjectBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT CameraConstantBufferSize = sizeof(FCameraConstantBuffer);
    BufferManager->CreateBufferGeneric<FCameraConstantBuffer>("FCameraConstantBuffer", nullptr, CameraConstantBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT SubUVBufferSize = sizeof(FSubUVConstant);
    BufferManager->CreateBufferGeneric<FSubUVConstant>("FSubUVConstant", nullptr, SubUVBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT MaterialBufferSize = sizeof(FMaterialConstants);
    BufferManager->CreateBufferGeneric<FMaterialConstants>("FMaterialConstants", nullptr, MaterialBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT SubMeshBufferSize = sizeof(FSubMeshConstants);
    BufferManager->CreateBufferGeneric<FSubMeshConstants>("FSubMeshConstants", nullptr, SubMeshBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT TextureBufferSize = sizeof(FTextureConstants);
    BufferManager->CreateBufferGeneric<FTextureConstants>("FTextureConstants", nullptr, TextureBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT LightingBufferSize = sizeof(FLightBuffer);
    BufferManager->CreateBufferGeneric<FLightBuffer>("FLightBuffer", nullptr, LightingBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT LitUnlitBufferSize = sizeof(FLitUnlitConstants);
    BufferManager->CreateBufferGeneric<FLitUnlitConstants>("FLitUnlitConstants", nullptr, LitUnlitBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT ViewModeBufferSize = sizeof(FViewModeConstants);
    BufferManager->CreateBufferGeneric<FViewModeConstants>("FViewModeConstants", nullptr, ViewModeBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT ScreenConstantsBufferSize = sizeof(FScreenConstants);
    BufferManager->CreateBufferGeneric<FScreenConstants>("FScreenConstants", nullptr, ScreenConstantsBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT FogConstantBufferSize = sizeof(FFogConstants);
    BufferManager->CreateBufferGeneric<FFogConstants>("FFogConstants", nullptr, FogConstantBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);


    // TODO: 함수로 분리
    ID3D11Buffer* ObjectBuffer = BufferManager->GetConstantBuffer(TEXT("FObjectConstantBuffer"));
    ID3D11Buffer* CameraConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FCameraConstantBuffer"));
    Graphics->DeviceContext->VSSetConstantBuffers(12, 1, &ObjectBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(13, 1, &CameraConstantBuffer);
    Graphics->DeviceContext->PSSetConstantBuffers(12, 1, &ObjectBuffer);
    Graphics->DeviceContext->PSSetConstantBuffers(13, 1, &CameraConstantBuffer);
}

void FRenderer::ReleaseConstantBuffer()
{
    BufferManager->ReleaseConstantBuffer();
}

void FRenderer::CreateCommonShader()
{
    D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"StaticMeshVertexShader", L"Shaders/StaticMeshVertexShader.hlsl", "mainVS", StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc));
}

void FRenderer::PrepareRender(FRenderTargetRHI* RenderTargetRHI)
{
    // Setup Viewport
    Graphics->DeviceContext->RSSetViewports(1, &RenderTargetRHI->GetD3DViewport());

    PrepareRenderPass();
}

void FRenderer::PrepareRenderPass()
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

void FRenderer::SetRenderResource(EResourceType Type, FRenderTargetRHI* RenderTargetRHI, bool bClearRTV, bool bIncludeDSV, bool bClearDSV)
{
    FViewportResources* ResourceRHI = RenderTargetRHI->Resources.Find(Type);
    if (!ResourceRHI)
    {
        if (FAILED(RenderTargetRHI->CreateResource(Type)))
        {
            return;
        }
        ResourceRHI = RenderTargetRHI->Resources.Find(Type);
    }
    
    Graphics->DeviceContext->OMSetRenderTargets(1, &ResourceRHI->RTV, bIncludeDSV ? RenderTargetRHI->DepthStencilView : nullptr);

    if (bClearRTV)
    {
        Graphics->DeviceContext->ClearRenderTargetView(ResourceRHI->RTV, RenderTargetRHI->GetClearColor(Type).data());
        if (bIncludeDSV && bClearDSV)
        {
            Graphics->DeviceContext->ClearDepthStencilView(RenderTargetRHI->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }
    }
}

void FRenderer::UpdateCommonBuffer(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FCameraConstantBuffer CameraConstantBuffer;
    CameraConstantBuffer.ViewMatrix = Viewport->GetViewMatrix();
    CameraConstantBuffer.InvViewMatrix = FMatrix::Inverse(CameraConstantBuffer.ViewMatrix);
    CameraConstantBuffer.ProjectionMatrix = Viewport->GetProjectionMatrix();
    CameraConstantBuffer.InvProjectionMatrix = FMatrix::Inverse(CameraConstantBuffer.ProjectionMatrix);
    CameraConstantBuffer.ViewLocation = Viewport->GetCameraLocation();
    CameraConstantBuffer.NearClip = Viewport->GetCameraLearClip();
    CameraConstantBuffer.FarClip = Viewport->GetCameraFarClip();
    BufferManager->UpdateConstantBuffer("FCameraConstantBuffer", CameraConstantBuffer);

    ID3D11Buffer* CameraBuffer = BufferManager->GetConstantBuffer("FCameraConstantBuffer");
    if (CameraBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        Graphics->DeviceContext->Map(CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
        if (FCameraConstantBuffer* Buffer = static_cast<FCameraConstantBuffer*>(MappedResource.pData))
        {
            Buffer->ViewMatrix = Viewport->GetViewMatrix();
            Buffer->InvViewMatrix = FMatrix::Inverse(CameraConstantBuffer.ViewMatrix);
            Buffer->ProjectionMatrix = Viewport->GetProjectionMatrix();
            Buffer->InvProjectionMatrix = FMatrix::Inverse(CameraConstantBuffer.ProjectionMatrix);
            Buffer->ViewLocation = Viewport->GetCameraLocation();
            Buffer->NearClip = Viewport->GetCameraLearClip();
            Buffer->FarClip = Viewport->GetCameraFarClip();
        }
        Graphics->DeviceContext->Unmap(CameraBuffer, 0);
    }
}

void FRenderer::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    if (!RenderTargetRHI)
    {
        return;
    }

    UpdateCommonBuffer(Viewport);
    
    PrepareRender(RenderTargetRHI);
    
    RenderWorldScene(Viewport);

    // RenderPostProcess(Viewport);
    
    // RenderEditorOverlay(Viewport);

    // Compositing
    CompositingPass->Render(Viewport);

    // Clear
    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(100, 1, NullSRV);

    ClearRenderArr();
}

void FRenderer::RenderWorldScene(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const uint64 ShowFlag = Viewport->GetShowFlag();
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    if (ShowFlag & EEngineShowFlags::SF_Primitives)
    {
        UpdateLightBufferPass->Render(Viewport);
        StaticMeshRenderPass->Render(Viewport);
    }
}

void FRenderer::RenderPostProcess(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const uint64 ShowFlag = Viewport->GetShowFlag();
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    
    if (ViewMode != EViewModeIndex::VMI_Lit)
    {
        return;
    }
    
    if (ShowFlag & EEngineShowFlags::SF_Fog)
    {
        // SetRenderResource(EResourceType::ERT_PP_Fog, RenderTargetRHI);
        // TODO: 여기에서는 씬 렌더가 적용된 뎁스 스텐실 뷰를 바인딩 해제해서 SRV로 전달하고, 뎁스 스텐실 뷰를 아래에서 다시 써야함.
    }
}

void FRenderer::RenderEditorOverlay(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const uint64 ShowFlag = Viewport->GetShowFlag();
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    
    // Render Billboard
    if (ShowFlag & EEngineShowFlags::SF_BillboardText)
    {
        SetRenderResource(EResourceType::ERT_Scene, RenderTargetRHI, false);
        BillboardRenderPass->Render(Viewport);
    }

    // 일단 수동으로 렌더타겟 해제하고 쉐이더 리소스 뷰에 씬 텍스처 전달
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(100, 1, &RenderTargetRHI->GetResources().Find(EResourceType::ERT_Scene)->SRV);

    // Render for Editor
    if (GEngine->ActiveWorld->WorldType == EWorldType::Editor)
    {
        SetRenderResource(EResourceType::ERT_Editor, RenderTargetRHI, true, true, false);
        LineRenderPass->Render(Viewport); // 기존 뎁스를 그대로 사용하지만 뎁스를 클리어하지는 않음

        FViewportResources* ResourceRHI = RenderTargetRHI->Resources.Find(EResourceType::ERT_Editor);
        Graphics->DeviceContext->ClearDepthStencilView(RenderTargetRHI->GizmoDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        Graphics->DeviceContext->OMSetRenderTargets(1, &ResourceRHI->RTV, RenderTargetRHI->GizmoDepthStencilView);
        GizmoRenderPass->Render(Viewport); // 기존 뎁스를 SRV로 전달해서 샘플 후 비교하기 위해 기즈모 전용 DSV 사용

        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        Graphics->DeviceContext->PSSetShaderResources(102, 1, &RenderTargetRHI->GetResources().Find(EResourceType::ERT_Editor)->SRV);
    }
}

void FRenderer::RenderViewport(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    SlateRenderPass->Render(Viewport);
}
