
#include "GizmoRenderPass.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "RendererHelpers.h"
#include "Math/JungleMath.h"

#include "World.h"

#include "Actors/Player.h"

#include "BaseGizmos/GizmoBaseComponent.h"

#include "UnrealEd/EditorViewportClient.h"

#include "PropertyEditor/ShowFlags.h"

#include "EngineLoop.h"

#include "UObject/ObjectTypes.h"

#include "Components/StaticMeshComponent.h"


// 생성자/소멸자
FGizmoRenderPass::FGizmoRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FGizmoRenderPass::~FGizmoRenderPass()
{
}

void FGizmoRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FGizmoRenderPass::PrepareRender()
{
    GizmoObjs.Empty();
    for (const auto iter : TObjectRange<UGizmoBaseComponent>())
    {
        GizmoObjs.Add(iter);
    }
}

void FGizmoRenderPass::Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& Viewport)
{

    for (UGizmoBaseComponent* GizmoComp : GizmoObjs)
    {
        auto controlMode = World->GetEditorPlayer()->GetControlMode();
        if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowZ) && controlMode != CM_TRANSLATION)
        {
            continue;
        }
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleZ) && controlMode != CM_SCALE)
        {
            continue;
        }
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleZ) && controlMode != CM_ROTATION)
        {
            continue;
        }

        ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
        Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
        Graphics->DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID);

        RenderGizmoComponent(GizmoComp, Viewport, World);

        Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
        Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
    }
}

void FGizmoRenderPass::RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& Viewport, const UWorld* World)
{
    bool selected = World->GetSelectedActor();
    if (!selected)
    {
        return;
    }
    // 모델 행렬 계산
    FMatrix Model = JungleMath::CreateModelMatrix(
        GizmoComp->GetWorldLocation(),
        GizmoComp->GetWorldRotation(),
        GizmoComp->GetWorldScale()
    );

    FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

    bool Selected = (GizmoComp == World->GetPickingGizmo());

    FMatrix MVP = RendererHelpers::CalculateMVP(Model, Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix());

    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);

    FPerObjectConstantBuffer Data(MVP, NormalMatrix, UUIDColor, Selected);

    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), Data);

    // Gizmo가 렌더링할 StaticMesh가 없으면 렌더링하지 않음
    if (!GizmoComp->GetStaticMesh()) return;

    OBJ::FStaticMeshRenderData* RenderData = GizmoComp->GetStaticMesh()->GetRenderData();

    if (!RenderData) return;

    UINT stride = sizeof(FVertexSimple);

    UINT offset = 0;

    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &stride, &offset);

    if (RenderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
    }
    else
    {
        for (int subMeshIndex = 0; subMeshIndex < RenderData->MaterialSubsets.Num(); subMeshIndex++)
        {
            int materialIndex = RenderData->MaterialSubsets[subMeshIndex].MaterialIndex;

            FSubMeshConstants SubMeshData = FSubMeshConstants(false);
            BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

            TArray<FStaticMaterial*>Materials = GizmoComp->GetStaticMesh()->GetMaterials();
            TArray<UMaterial*>OverrideMaterials = GizmoComp->GetOverrideMaterials();
            
            if (OverrideMaterials[materialIndex] != nullptr)
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[materialIndex]->GetMaterialInfo());
            else
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[materialIndex]->Material->GetMaterialInfo());

            uint64 startIndex = RenderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = RenderData->MaterialSubsets[subMeshIndex].IndexCount;

            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }
    }
}


void FGizmoRenderPass::ClearRenderArr()
{
    GizmoObjs.Empty();
}
