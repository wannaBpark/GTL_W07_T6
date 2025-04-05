#include "Define.h"
#include "LightRenderPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/LightComponent.h"
#include "Math/JungleMath.h"

#include "World.h"
#include "EngineLoop.h"

#include "UObject/UObjectIterator.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FLightRenderPass::FLightRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FLightRenderPass::~FLightRenderPass()
{
}

void FLightRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FLightRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        LightObjs.Add(iter);
    }
}

void FLightRenderPass::Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& Viewport)
{

    // 각 라이트에 대해 디버깅용 렌더링 수행 (예: 원뿔, OBB)
    for (auto Light : LightObjs)
    {
        // 월드 변환 행렬 계산 (스케일 1로 가정)
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });

        // 디버깅용 원뿔 렌더링: 라이트 위치, 반지름, 분할 수, 각도, 색상, 모델 행렬
        FEngineLoop::PrimitiveDrawBatch.AddConeToBatch(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);

        // 디버깅용 OBB 렌더링: 라이트의 바운딩 박스, 위치, 모델 행렬
        FEngineLoop::PrimitiveDrawBatch.AddOBBToBatch(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}

void FLightRenderPass::ClearRenderArr()
{
    LightObjs.Empty();
}

void FLightRenderPass::UpdateLightBuffer() const
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