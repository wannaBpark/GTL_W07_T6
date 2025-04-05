
#include "Renderer.h"

#include "World.h"

#include "Actors/Player.h"

#include "BaseGizmos/GizmoBaseComponent.h"

#include "Components/LightComponent.h"

#include "Components/UBillboardComponent.h"

#include "Components/UParticleSubUVComp.h"

#include "Components/UText.h"

#include "Launch/EngineLoop.h"

#include "Math/JungleMath.h"

#include "UnrealEd/EditorViewportClient.h"

#include "UObject/Casts.h"

#include "UObject/Object.h"

#include "PropertyEditor/ShowFlags.h"

#include "UObject/UObjectIterator.h"

#include "D3D11RHI/DXDShaderManager.h"

#include "D3D11RHI/DXDBufferManager.h"

#include "RendererHelpers.h"

#include "StaticMeshRenderPass.h"

//------------------------------------------------------------------------------
// 초기화 및 해제 관련 함수
//------------------------------------------------------------------------------
void FRenderer::Initialize(FGraphicsDevice* InGraphics, FDXDBufferManager* InBufferManager)
{
    Graphics = InGraphics;

    ShaderManager = new FDXDShaderManager(Graphics->Device);
    StaticMeshRenderPass = new FStaticMeshRenderPass();

    BufferManager = InBufferManager;

    StaticMeshRenderPass->Initialize(BufferManager, Graphics, ShaderManager);

    StaticMeshRenderPass->CreateShader();

    CreateShader();

    CreateConstantBuffer();
}

void FRenderer::Release()
{
    ReleaseLineShader();
    ReleaseTextureShader();

}

//------------------------------------------------------------------------------
// 셰이더 생성 및 해제 함수
//------------------------------------------------------------------------------
void FRenderer::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC TexturelayoutDesc[] = {
       {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
       {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    TextureStride = sizeof(FVertexTexture);


    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"VertexBillBoardShader", L"Shaders/VertexBillBoardShader.hlsl", "main", TexturelayoutDesc, ARRAYSIZE(TexturelayoutDesc));

    hr = ShaderManager->AddPixelShader(L"PixelBillBoardShader", L"Shaders/PixelBillBoardShader.hlsl", "main");

    hr = ShaderManager->AddVertexShader(L"VertexLineShader", L"Shaders/ShaderLine.hlsl", "mainVS");

    hr = ShaderManager->AddPixelShader(L"PixelLineShader", L"Shaders/ShaderLine.hlsl", "mainPS");

    VertexTextureShader = ShaderManager->GetVertexShaderByKey(L"VertexBillBoardShader");

    PixelBillBoardShader = ShaderManager->GetPixelShaderByKey(L"PixelBillBoardShader");

    BillBoardInputLayout = ShaderManager->GetInputLayoutByKey(L"VertexBillBoardShader");

    VertexLineShader = ShaderManager->GetVertexShaderByKey(L"VertexLineShader");

    PixelLineShader = ShaderManager->GetPixelShaderByKey(L"PixelLineShader");
}

void FRenderer::ChangeViewMode(EViewModeIndex evi) const
{

}

//------------------------------------------------------------------------------
// 상수 버퍼 및 기타 버퍼 생성/해제 함수
//------------------------------------------------------------------------------
void FRenderer::CreateConstantBuffer()
{
    UINT perObjectBufferSize = sizeof(FPerObjectConstantBuffer);
    BufferManager->CreateBufferGeneric<FPerObjectConstantBuffer>("FPerObjectConstantBuffer", nullptr, perObjectBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT subUVBufferSize = sizeof(FSubUVConstant);
    BufferManager->CreateBufferGeneric<FSubUVConstant>("FSubUVConstant", nullptr, subUVBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT materialBufferSize = sizeof(FMaterialConstants);
    BufferManager->CreateBufferGeneric<FMaterialConstants>("FMaterialConstants", nullptr, materialBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT subMeshBufferSize = sizeof(FSubMeshConstants);
    BufferManager->CreateBufferGeneric<FSubMeshConstants>("FSubMeshConstants", nullptr, subMeshBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT textureBufferSize = sizeof(FTextureConstants);
    BufferManager->CreateBufferGeneric<FTextureConstants>("FTextureConstants", nullptr, textureBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT lightingBufferSize = sizeof(FLighting);
    BufferManager->CreateBufferGeneric<FLighting>("FLighting", nullptr, lightingBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT litUnlitBufferSize = sizeof(FLitUnlitConstants);
    BufferManager->CreateBufferGeneric<FLitUnlitConstants>("FLitUnlitConstants", nullptr, litUnlitBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void FRenderer::ReleaseConstantBuffer()
{
    BufferManager->ReleaseConstantBuffer();
}

//------------------------------------------------------------------------------
// 업데이트 함수 (상수 버퍼, 재질, 텍스처 등)
//------------------------------------------------------------------------------
void FRenderer::UpdateLightBuffer() const
{
    FLighting data;
    data.lightDirX = 1.0f;
    data.lightDirY = 1.0f;
    data.lightDirZ = 1.0f;
    data.lightColorX = 1.0f;
    data.lightColorY = 1.0f;
    data.lightColorZ = 1.0f;
    data.AmbientFactor = 0.06f;

    BufferManager->UpdateConstantBuffer(TEXT("FLighting"), data);
}

void FRenderer::UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const
{
    FMaterialConstants data;
    data.DiffuseColor = MaterialInfo.Diffuse;
    data.TransparencyScalar = MaterialInfo.TransparencyScalar;
    data.AmbientColor = MaterialInfo.Ambient;
    data.DensityScalar = MaterialInfo.DensityScalar;
    data.SpecularColor = MaterialInfo.Specular;
    data.SpecularScalar = MaterialInfo.SpecularScalar;
    data.EmmisiveColor = MaterialInfo.Emissive;


    BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), data);

    if (MaterialInfo.bHasTexture)
    {
        std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
        Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        ID3D11SamplerState* nullSampler[1] = { nullptr };
        Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
    }
}

void FRenderer::UpdateLitUnlitConstant(int isLit) const
{
    FLitUnlitConstants data;
    data.isLit = isLit;
    BufferManager->UpdateConstantBuffer(TEXT("FLitUnlitConstants"), data);
}

void FRenderer::UpdateTextureConstant(float UOffset, float VOffset) const
{
    FTextureConstants data;
    data.UOffset = UOffset;
    data.VOffset = VOffset;

    BufferManager->UpdateConstantBuffer(TEXT("FTextureConstants"), data);
}

void FRenderer::UpdateSubUVConstant(float _indexU, float _indexV) const
{
    FSubUVConstant data;
    data.indexU = _indexU;
    data.indexV = _indexV;
    BufferManager->UpdateConstantBuffer(TEXT("FSubUVConstantBuffer"), data);
}

void FRenderer::PrepareSubUVConstant() const
{
    BufferManager->BindConstantBuffer(TEXT("FSubUVConstantBuffer"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FSubUVConstantBuffer"), 0, EShaderStage::Pixel);
}

void FRenderer::UpdatePerObjectConstant(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FVector4& UUIDColor, bool selected) const
{
    FMatrix MVP = RendererHelpers::CalculateMVP(Model, View, Projection);
    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);
    FPerObjectConstantBuffer data(MVP, NormalMatrix, UUIDColor, selected);
    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), data);
}

//------------------------------------------------------------------------------
// 텍스처 셰이더 관련 함수
//------------------------------------------------------------------------------

void FRenderer::ReleaseTextureShader()
{
    FDXDBufferManager::SafeRelease(BillBoardInputLayout);
    FDXDBufferManager::SafeRelease(PixelBillBoardShader);
    FDXDBufferManager::SafeRelease(VertexTextureShader);

}

void FRenderer::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexTextureShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelBillBoardShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(BillBoardInputLayout);

    BufferManager->BindConstantBuffer(TEXT("FPerObjectConstantBuffer"), 0, EShaderStage::Vertex);
}

void FRenderer::RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FRenderer::RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

//------------------------------------------------------------------------------
// Line 셰이더 관련 함수
//------------------------------------------------------------------------------

void FRenderer::ReleaseLineShader()
{
    FDXDBufferManager::SafeRelease(VertexLineShader);
    FDXDBufferManager::SafeRelease(PixelLineShader);
}

void FRenderer::PrepareLineShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);

    BufferManager->BindConstantBuffer(TEXT("FPerObjectConstantBuffer"), 0, EShaderStage::Vertex);
    FEngineLoop::PrimitiveDrawBatch.PrepareLineResources();
}

void FRenderer::ProcessLineRendering(const FMatrix& View, const FMatrix& Projection)
{
    PrepareLineShader();

    UpdatePerObjectConstant(FMatrix::Identity, View, Projection, FVector4(0, 0, 0, 0), false);

    FLinePrimitiveBatchArgs LinePrimitveBatchArgs;

    FEngineLoop::PrimitiveDrawBatch.PrepareBatch(LinePrimitveBatchArgs);

    DrawLineBatch(LinePrimitveBatchArgs);

    FEngineLoop::PrimitiveDrawBatch.RemoveArr();
}

void FRenderer::DrawLineBatch(const FLinePrimitiveBatchArgs& linePrimitiveBatchArgs) const
{
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &linePrimitiveBatchArgs.VertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    UINT vertexCountPerInstance = 2;
    UINT instanceCount = linePrimitiveBatchArgs.GridParam.NumGridLines + 3 +
        (linePrimitiveBatchArgs.BoundingBoxCount * 12) +
        (linePrimitiveBatchArgs.ConeCount * (2 * linePrimitiveBatchArgs.ConeSegmentCount)) +
        (12 * linePrimitiveBatchArgs.OBBCount);
    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//------------------------------------------------------------------------------
// 렌더 패스 관련 함수
//------------------------------------------------------------------------------
void FRenderer::PrepareRender()
{
    StaticMeshRenderPass->PrepareRender();

    for (const auto iter : TObjectRange<USceneComponent>())
    {
        if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
        {
            GizmoObjs.Add(pGizmoComp);
        }
        if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
        {
            BillboardObjs.Add(pBillboardComp);
        }
        if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
        {
            LightObjs.Add(pLightComp);
        }
    }
}

void FRenderer::ClearRenderArr()
{
    StaticMeshRenderPass->ClearRenderArr();
    GizmoObjs.Empty();
    BillboardObjs.Empty();
    LightObjs.Empty();
}

void FRenderer::Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());

    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());

    ChangeViewMode(ActiveViewport->GetViewMode());

    UpdateLightBuffer();

    ProcessLineRendering(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    StaticMeshRenderPass->Render(World, ActiveViewport);

    RenderGizmos(World, ActiveViewport);

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
        RenderBillboards(World, ActiveViewport);

    RenderLight(World, ActiveViewport);

    ClearRenderArr();
}

bool FRenderer::ShouldRenderGizmo(UGizmoBaseComponent* GizmoComp, const UWorld* World) const
{
    auto controlMode = World->GetEditorPlayer()->GetControlMode();
    if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowX ||
        GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowY ||
        GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowZ) &&
        controlMode != CM_TRANSLATION)
    {
        return false;
    }
    else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleX ||
        GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleY ||
        GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleZ) &&
        controlMode != CM_SCALE)
    {
        return false;
    }
    else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleX ||
        GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleY ||
        GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleZ) &&
        controlMode != CM_ROTATION)
    {
        return false;
    }
    return true;
}

// 각 Gizmo 컴포넌트에 대한 렌더링 작업을 수행하는 함수
void FRenderer::RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& ActiveViewport, const UWorld* World)
{
    FMatrix Model = JungleMath::CreateModelMatrix(
        GizmoComp->GetWorldLocation(),
        GizmoComp->GetWorldRotation(),
        GizmoComp->GetWorldScale()
    );

    FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

    bool selected = (GizmoComp == World->GetPickingGizmo());

    UpdatePerObjectConstant(Model, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), UUIDColor, selected);

    if (!GizmoComp->GetStaticMesh()) return;

    OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();

    if (!renderData) return;

    StaticMeshRenderPass->RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials(), -1);
}

void FRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!World->GetSelectedActor())
        return;

    // 상태 설정
    ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    Graphics->DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID);

    for (auto GizmoComp : GizmoObjs)
    {
        if (!ShouldRenderGizmo(GizmoComp, World)) continue;

        RenderGizmoComponent(GizmoComp, ActiveViewport, World);
    }

    // 상태 복원
    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
}


void FRenderer::RenderBillboards(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    PrepareTextureShader();
    PrepareSubUVConstant();
    for (auto BillboardComp : BillboardObjs)
    {
        UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();

        FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;

        if (BillboardComp == World->GetPickingGizmo())
            UpdatePerObjectConstant(Model, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), UUIDColor, true);
        else
            UpdatePerObjectConstant(Model, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), UUIDColor, false);


        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices, SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState);

        }
        else if (UText* Text = Cast<UText>(BillboardComp))
        {
            RenderTextPrimitive(Text->vertexTextBuffer, Text->numTextVertices, Text->Texture->TextureSRV, Text->Texture->SamplerState);
        }
        else
        {
            RenderTexturePrimitive(BillboardComp->vertexTextureBuffer, BillboardComp->numVertices, BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState);
        }
    }
}

void FRenderer::RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : LightObjs) {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });
        FEngineLoop::PrimitiveDrawBatch.AddConeToBatch(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        FEngineLoop::PrimitiveDrawBatch.AddOBBToBatch(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}
