#include "LineRenderPass.h"

#include "D3D11RHI/DXDBufferManager.h"

#include "D3D11RHI/GraphicDevice.h"

#include "D3D11RHI/DXDShaderManager.h"

#include "RendererHelpers.h"

#include "Math/JungleMath.h"

#include "EngineLoop.h"

#include "UObject/UObjectIterator.h"

#include "UnrealEd/EditorViewportClient.h"

// 생성자/소멸자
FLineRenderPass::FLineRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
    , VertexLineShader(nullptr)
    , PixelLineShader(nullptr)
{
}

FLineRenderPass::~FLineRenderPass()
{
    // 셰이더 자원 해제
    FDXDBufferManager::SafeRelease(VertexLineShader);
    FDXDBufferManager::SafeRelease(PixelLineShader);
}

void FLineRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    CreateShader();
}

void FLineRenderPass::PrepareRender()
{
    // 라인 렌더 패스에서는 별도의 렌더 대상 수집이 필요하지 않은 경우,
    // 필요한 경우 TObjectRange 등을 통해 관련 데이터를 수집할 수 있습니다.
}

void FLineRenderPass::ClearRenderArr()
{
    // 필요에 따라 내부 배열을 초기화
}

void FLineRenderPass::CreateShader()
{
    HRESULT hr = ShaderManager->AddVertexShader(L"VertexLineShader", L"Shaders/ShaderLine.hlsl", "mainVS");
    hr = ShaderManager->AddPixelShader(L"PixelLineShader", L"Shaders/ShaderLine.hlsl", "mainPS");

    VertexLineShader = ShaderManager->GetVertexShaderByKey(L"VertexLineShader");
    PixelLineShader = ShaderManager->GetPixelShaderByKey(L"PixelLineShader");
}

void FLineRenderPass::PrepareLineShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);

    // 상수 버퍼 바인딩 (예: FPerObjectConstantBuffer)
    BufferManager->BindConstantBuffer(TEXT("FPerObjectConstantBuffer"), 0, EShaderStage::Vertex);

    // PrimitiveDrawBatch를 통해 라인 렌더링에 필요한 상태를 준비합니다.
    FEngineLoop::PrimitiveDrawBatch.PrepareLineResources();
}

void FLineRenderPass::DrawLineBatch(const FLinePrimitiveBatchArgs& BatchArgs) const
{
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &BatchArgs.VertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    const UINT vertexCountPerInstance = 2;
    UINT instanceCount = BatchArgs.GridParam.NumGridLines + 3 +
        (BatchArgs.BoundingBoxCount * 12) +
        (BatchArgs.ConeCount * (2 * BatchArgs.ConeSegmentCount)) +
        (12 * BatchArgs.OBBCount);

    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void FLineRenderPass::ProcessLineRendering(const FMatrix& View, const FMatrix& Projection)
{
    PrepareLineShader();

    // 상수 버퍼 업데이트: Identity 모델, 기본 색상 등
    FMatrix MVP = RendererHelpers::CalculateMVP(FMatrix::Identity, View, Projection);
    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(FMatrix::Identity);
    FPerObjectConstantBuffer Data(MVP, NormalMatrix, FVector4(0, 0, 0, 0), false);
    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), Data);

    FLinePrimitiveBatchArgs BatchArgs;
    FEngineLoop::PrimitiveDrawBatch.PrepareBatch(BatchArgs);
    DrawLineBatch(BatchArgs);
    FEngineLoop::PrimitiveDrawBatch.RemoveArr();
}

void FLineRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, Graphics->DepthStencilView);

    ProcessLineRendering(Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix());
}
