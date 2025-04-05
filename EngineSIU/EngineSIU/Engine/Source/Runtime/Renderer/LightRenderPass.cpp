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
    FLightBuffer LightBufferData = {};
    int LightCount = 0;

    LightBufferData.GlobalAmbientLight = FVector4(0.1, 0.1, 0.1,1);
    for (auto Light : LightObjs)
    {
        // 월드 변환 행렬 계산 (스케일 1로 가정)
        //FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });

        //FEngineLoop::PrimitiveDrawBatch.AddConeToBatch(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);

        //FEngineLoop::PrimitiveDrawBatch.AddOBBToBatch(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);

        if (LightCount < MAX_LIGHTS) //TODO LightObj가 정보를 갖도록 분리
        {
            LightBufferData.gLights[LightCount] = FLight{};
            LightBufferData.gLights[LightCount] = Light->GetLightInfo();
            LightBufferData.gLights[LightCount].Position = Light->GetWorldLocation();
            LightBufferData.gLights[LightCount].Range = 100.f;
            LightBufferData.gLights[LightCount].AmbientColor = FVector(0.5,0.5,0.5);
            LightBufferData.gLights[LightCount].DiffuseColor = FVector(0.1,0.1,0.1);
            LightBufferData.gLights[LightCount].Attenuation = FVector(0.5f,0.5f,0.5f);
            LightBufferData.gLights[LightCount].Direction = FVector(0.f,0.f,0.f);
            LightBufferData.gLights[LightCount].Type = 1;
            LightBufferData.gLights[LightCount].Falloff = 0.4f;

            LightBufferData.gLights[LightCount].Enabled = 1;

            LightCount++;
        } 

    }
    LightBufferData.nLights = LightCount;

    BufferManager->UpdateConstantBuffer(TEXT("FLightBuffer"), LightBufferData);
}    

void FLightRenderPass::ClearRenderArr()
{
    LightObjs.Empty();
}

void FLightRenderPass::UpdateLightBuffer(FLight Light) const
{

}