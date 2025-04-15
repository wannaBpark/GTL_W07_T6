
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


struct FViewportResources;
// 생성자/소멸자
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
    D3D11_INPUT_ELEMENT_DESC GizmoInputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"GizmoVertexShader", L"Shaders/GizmoVertexShader.hlsl", "mainVS", GizmoInputLayout, ARRAYSIZE(GizmoInputLayout));

    hr = ShaderManager->AddPixelShader(L"GizmoPixelShader", L"Shaders/GizmoPixelShader.hlsl", "mainPS");

    VertexShader = ShaderManager->GetVertexShaderByKey(L"GizmoVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"GizmoPixelShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"GizmoVertexShader");

}
void FGizmoRenderPass::ReleaseShader()
{
    FDXDBufferManager::SafeRelease(InputLayout);
    FDXDBufferManager::SafeRelease(PixelShader);
    FDXDBufferManager::SafeRelease(VertexShader);
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

    // 상수 버퍼 바인딩 예시
    ID3D11Buffer* PerObjectBuffer = BufferManager->GetConstantBuffer(TEXT("FPerObjectConstantBuffer"));
    ID3D11Buffer* CameraConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FCameraConstantBuffer"));
    Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &PerObjectBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &CameraConstantBuffer);

    TArray<FString> PSBufferKeys = {
        TEXT("FPerObjectConstantBuffer"),
        TEXT("FMaterialConstants"),
        TEXT("FLitUnlitConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);
}

void FGizmoRenderPass::PrepareRender()
{
}

void FGizmoRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerSolidBack);
    
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        UE_LOG(LogLevel::Error, TEXT("Gizmo RenderPass : Render : Engine is not valid."));
        return;
    }
    
    UWorld* ActiveWorld = GEngine->ActiveWorld;
    if (!ActiveWorld)
    {
        UE_LOG(LogLevel::Error, TEXT("Gizmo RenderPass : Render : ActiveWorld is not valid."));
        return;
    }
    
    ControlMode Mode = Engine->GetEditorPlayer()->GetControlMode();
    if (Mode == CM_TRANSLATION)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetArrowArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            RenderGizmoComponent(GizmoComp, Viewport, ActiveWorld);
        }
    }
    else if (Mode == CM_SCALE)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetScaleArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            RenderGizmoComponent(GizmoComp, Viewport, ActiveWorld);
        }
    }
    else if (Mode == CM_ROTATION)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetDiscArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            RenderGizmoComponent(GizmoComp, Viewport, ActiveWorld);
        }
    }
    
    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
}

void FGizmoRenderPass::RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& Viewport, const UWorld* World)
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
    
    // 모델 행렬.
    FMatrix Model = GizmoComp->GetWorldMatrix();
    FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;
    bool bSelected = (GizmoComp == Viewport->GetPickedGizmoComponent());
    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);

    FPerObjectConstantBuffer ObjectData(Model, NormalMatrix, UUIDColor, bSelected);

    FCameraConstantBuffer CameraData(Viewport->View, Viewport->Projection);

    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), ObjectData);
    BufferManager->UpdateConstantBuffer(TEXT("FCameraConstantBuffer"), CameraData);

    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (!RenderData->IndexBuffer)
    {
        // TODO: 인덱스 버퍼가 없는 경우를 따로 고려해야 함.
    }
    Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    
    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
    }
    else
    {
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
