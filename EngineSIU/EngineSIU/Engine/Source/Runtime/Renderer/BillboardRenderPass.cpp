#include "BillboardRenderPass.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "RendererHelpers.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "UnrealEd/EditorViewportClient.h"
#include "PropertyEditor/ShowFlags.h"

#include "Components/UBillboardComponent.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/UText.h"

#include "EngineLoop.h"

#include "World/World.h"

FBillboardRenderPass::FBillboardRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
    , VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , Stride(0)
{
}

FBillboardRenderPass::~FBillboardRenderPass()
{
    ReleaseShader();
}

void FBillboardRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FBillboardRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<UBillboardComponent>())
    {
        BillboardObjs.Add(iter);
    }
}

void FBillboardRenderPass::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    BufferManager->BindConstantBuffer(TEXT("FPerObjectConstantBuffer"), 0, EShaderStage::Vertex);
}

void FBillboardRenderPass::PrepareSubUVConstant() const
{
    BufferManager->BindConstantBuffer(TEXT("FSubUVConstantBuffer"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FSubUVConstantBuffer"), 0, EShaderStage::Pixel);
}

void FBillboardRenderPass::UpdateSubUVConstant(float _indexU, float _indexV) const
{
    FSubUVConstant data;
    data.indexU = _indexU;
    data.indexV = _indexV;
    BufferManager->UpdateConstantBuffer(TEXT("FSubUVConstantBuffer"), data);
}

void FBillboardRenderPass::UpdatePerObjectConstant(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FVector4& UUIDColor, bool Selected) const
{
    FMatrix MVP = RendererHelpers::CalculateMVP(Model, View, Projection);
    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);
    FPerObjectConstantBuffer data(MVP, NormalMatrix, UUIDColor, Selected);
    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), data);
}

void FBillboardRenderPass::RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    ID3D11Buffer* pIndexBuffer, UINT numIndices,
    ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FBillboardRenderPass::RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FBillboardRenderPass::CreateShader()
{
    // Billboard 셰이더 생성
    D3D11_INPUT_ELEMENT_DESC TextureLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Stride = sizeof(FVertexTexture);

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"VertexBillboardShader", L"Shaders/VertexBillboardShader.hlsl", "main", TextureLayoutDesc, ARRAYSIZE(TextureLayoutDesc));
    hr = ShaderManager->AddPixelShader(L"PixelBillboardShader", L"Shaders/PixelBillboardShader.hlsl", "main");

    VertexShader = ShaderManager->GetVertexShaderByKey(L"VertexBillboardShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"PixelBillboardShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"VertexBillboardShader");
}

void FBillboardRenderPass::ReleaseShader()
{
    FDXDBufferManager::SafeRelease(InputLayout);
    FDXDBufferManager::SafeRelease(PixelShader);
    FDXDBufferManager::SafeRelease(VertexShader);
}

void FBillboardRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    if (!(Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))) return;
    PrepareTextureShader();
    PrepareSubUVConstant();

    // 각 Billboard에 대해 렌더링 처리
    for (auto BillboardComp : BillboardObjs)
    {
        UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();
        FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
        bool Selected = (BillboardComp == Viewport->GetPickedGizmoComponent());
        UpdatePerObjectConstant(Model, Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix(), UUIDColor, Selected);

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(SubUVParticle->vertexSubUVBuffer.Get(), SubUVParticle->numTextVertices,
                SubUVParticle->indexTextureBuffer.Get(), SubUVParticle->numIndices,
                SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState);
        }
        else if (UText* Text = Cast<UText>(BillboardComp))
        {
            RenderTextPrimitive(Text->vertexTextBuffer.Get(), Text->numTextVertices,
                Text->Texture->TextureSRV, Text->Texture->SamplerState);
        }
        else
        {
            RenderTexturePrimitive(BillboardComp->vertexTextureBuffer.Get(), BillboardComp->numVertices,
                BillboardComp->indexTextureBuffer.Get(), BillboardComp->numIndices,
                BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState);
        }
    }
}

void FBillboardRenderPass::Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // TODO: 제거 필요 Interface에서 직접 제거.
}

void FBillboardRenderPass::ClearRenderArr()
{
    BillboardObjs.Empty();
}
