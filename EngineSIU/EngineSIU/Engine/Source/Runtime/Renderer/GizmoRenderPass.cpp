
#include "GizmoRenderPass.h"

#include <array>

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "RendererHelpers.h"
#include "Math/JungleMath.h"

#include "World/World.h"

#include "Actors/Player.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "BaseGizmos/TransformGizmo.h"

#include "UnrealEd/EditorViewportClient.h"

#include "PropertyEditor/ShowFlags.h"

#include "EngineLoop.h"
#include "UnrealClient.h"

#include "UObject/ObjectTypes.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/EditorEngine.h"


FGizmoRenderPass::FGizmoRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FGizmoRenderPass::~FGizmoRenderPass()
{
    ReleaseShader();
    if (ShaderManager)
    {
        delete ShaderManager;
        ShaderManager = nullptr;
    }
}

void FGizmoRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    
    CreateShader();
}

void FGizmoRenderPass::CreateShader()
{
    VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");

    HRESULT hr = ShaderManager->AddPixelShader(L"GizmoPixelShader", L"Shaders/GizmoPixelShader.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }
    PixelShader = ShaderManager->GetPixelShaderByKey(L"GizmoPixelShader");
}

void FGizmoRenderPass::ReleaseShader()
{
}

void FGizmoRenderPass::ClearRenderArr()
{
}

void FGizmoRenderPass::PrepareRenderState() const
{
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    ID3D11Buffer* MaterialConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FMaterialConstants"));
    Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &MaterialConstantBuffer);
    // TODO: 뷰포트 크기 버퍼 생성 및 바인딩
}

void FGizmoRenderPass::PrepareRender()
{
}

void FGizmoRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetRenderTargetRHI();
    FViewportResources* ResourceRHI = RenderTargetRHI->GetResource(EResourceType::ERT_Editor);
    Graphics->DeviceContext->ClearDepthStencilView(RenderTargetRHI->GetGizmoDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    Graphics->DeviceContext->OMSetRenderTargets(1, &ResourceRHI->RTV, RenderTargetRHI->GetGizmoDepthStencilView());
    Graphics->DeviceContext->PSSetShaderResources(99, 1, &RenderTargetRHI->GetDepthStencilSRV());
    
    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerSolidBack);
    
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        UE_LOG(LogLevel::Error, TEXT("Gizmo RenderPass : Render : Engine is not valid."));
        return;
    }
    
    ControlMode Mode = Engine->GetEditorPlayer()->GetControlMode();
    if (Mode == CM_TRANSLATION)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetArrowArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            RenderGizmoComponent(GizmoComp, Viewport);
        }
    }
    else if (Mode == CM_SCALE)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetScaleArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            RenderGizmoComponent(GizmoComp, Viewport);
        }
    }
    else if (Mode == CM_ROTATION)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetDiscArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            RenderGizmoComponent(GizmoComp, Viewport);
        }
    }
    
    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(99, 1, NullSRV);
}

void FGizmoRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;
    
    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FGizmoRenderPass::RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (Engine && !Engine->GetSelectedActor())
    {
        return;
    }
    if (!GizmoComp->GetStaticMesh())
    {
        return;
    }
    
    OBJ::FStaticMeshRenderData* RenderData = GizmoComp->GetStaticMesh()->GetRenderData();
    if (!RenderData)
    {
        return;
    }

    PrepareRenderState();
    
    // 오브젝트 버퍼 업데이트
    FMatrix WorldMatrix = GizmoComp->GetWorldMatrix();
    FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;
    bool bIsSelected = (GizmoComp == Viewport->GetPickedGizmoComponent());
    UpdateObjectConstant(WorldMatrix, UUIDColor, bIsSelected);

    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (!RenderData->IndexBuffer)
    {
        // TODO: 인덱스 버퍼가 없는 경우?
    }
    Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    
    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
    }
    else
    {
        // TODO: 현재 기즈모 메시는 머티리얼이 하나뿐이지만, 추후 여러 머티리얼을 사용하는 경우가 생길 수 있음.
        for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
        {
            int32 MaterialIndex = RenderData->MaterialSubsets[SubMeshIndex].MaterialIndex;

            FSubMeshConstants SubMeshData = FSubMeshConstants(false);
            BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

            TArray<UMaterial*> OverrideMaterials = GizmoComp->GetOverrideMaterials();
            if (OverrideMaterials[MaterialIndex] != nullptr)
            {
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[MaterialIndex]->GetMaterialInfo());
            }
            else
            {
                TArray<FStaticMaterial*> Materials = GizmoComp->GetStaticMesh()->GetMaterials();
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[MaterialIndex]->Material->GetMaterialInfo());
            }

            uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
            uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;

            Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
        }
    }
}
