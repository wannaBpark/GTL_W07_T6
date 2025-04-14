#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AmbientLightComponent.h"
#include "Math/JungleMath.h"
#include "Engine/EditorEngine.h"
#include "World/World.h"
#include "EngineLoop.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FUpdateLightBufferPass::~FUpdateLightBufferPass()
{
}

void FUpdateLightBufferPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FUpdateLightBufferPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
            else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
            else if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
            {
                DirectionalLights.Add(DirectionalLight);
            }
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    UpdateLightBuffer();
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    PointLights.Empty();
    SpotLights.Empty();
    DirectionalLights.Empty();
}


void FUpdateLightBufferPass::UpdateLightBuffer() const
{
    FLightInfoBuffer LightBufferData = {};
   

    int DirectionalLightsCount=0;
    int PointLightsCount=0;
    int SpotLightsCount=0;
    
    for (auto Light : SpotLights)
    {
        if (SpotLightsCount < MAX_SPOT_LIGHT)
        {
            LightBufferData.SpotLights[SpotLightsCount] = Light->GetSpotLightInfo();
            LightBufferData.SpotLights[SpotLightsCount].Position = Light->GetWorldLocation();
            LightBufferData.SpotLights[SpotLightsCount].Direction = Light->GetDirection();
            SpotLightsCount++;
        }
    }

    for (auto Light : PointLights)
    {
        if (PointLightsCount < MAX_POINT_LIGHT)
        {
            LightBufferData.PointLights[PointLightsCount] = Light->GetPointLightInfo();
            LightBufferData.PointLights[PointLightsCount].Position = Light->GetWorldLocation();
            PointLightsCount++;
        }
    }
    
    for (auto Light : DirectionalLights)
    {
        if (DirectionalLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            LightBufferData.Directional[DirectionalLightsCount] = Light->GetDirectionalLightInfo();
            LightBufferData.Directional[DirectionalLightsCount].Direction = Light->GetDirection();
            DirectionalLightsCount++;
        }
    }
    
    //LightBufferData.Ambient = AmbientLights->GetAmbientLightInfo();
    FAmbientLightInfo ambient;
    ambient.AmbientColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    LightBufferData.Ambient = ambient;
    LightBufferData.DirectionalLightsCount = DirectionalLightsCount;
    LightBufferData.PointLightsCount = PointLightsCount;
    LightBufferData.SpotLightsCount = SpotLightsCount;

    BufferManager->UpdateConstantBuffer(TEXT("FLightInfoBuffer"), LightBufferData);
    
}

