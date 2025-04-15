#include "TileLightCullingPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

FTileLightCullingPass::FTileLightCullingPass()
{
}

FTileLightCullingPass::~FTileLightCullingPass()
{
}

void FTileLightCullingPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;
    
    TILE_COUNT_X = (Graphics->screenWidth + TILE_SIZE - 1) / TILE_SIZE;
    TILE_COUNT_Y = (Graphics->screenHeight + TILE_SIZE - 1) / TILE_SIZE;
    TILE_COUNT = TILE_COUNT_X * TILE_COUNT_Y;
    SHADER_ENTITY_TILE_BUCKET_COUNT = MAX_LIGHTS_PER_TILE / sizeof(uint32);
    // 한 타일이 가질 수 있는 조명 ID 목록을 비트마스크로 표현한 총 슬롯 수

    CreateShader();
    CreateViews();
    CreateBuffers();
}

void FTileLightCullingPass::PrepareRender()
{
}

void FTileLightCullingPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FTileLightCullingPass::ClearRenderArr()
{
}

void FTileLightCullingPass::CreateShader()
{
    // Compute Shader 생성
    HRESULT hr = ShaderManager->AddComputeShader(L"TileLightCullingComputeShader", L"Shaders/TileLightCullingComputeShader.hlsl", "mainCS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to Compile Compute Shader!"));
    }
    ComputeShader = ShaderManager->GetComputeShaderByKey(L"TileLightCullingComputeShader");

}

void FTileLightCullingPass::CreateViews()
{
    // 2. entityTiles UAV buffer (tile mask 결과용)
    D3D11_BUFFER_DESC tileBufferDesc = {};
    tileBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    tileBufferDesc.ByteWidth = sizeof(uint32) * TILE_COUNT * SHADER_ENTITY_TILE_BUCKET_COUNT;
    tileBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    tileBufferDesc.StructureByteStride = sizeof(uint32);
    tileBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HRESULT hr = Graphics->Device->CreateBuffer(&tileBufferDesc, nullptr, &TileUAVBuffer);

    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Tile UAV Buffer!"));
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC tileUAVDesc = {};
    tileUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    tileUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    tileUAVDesc.Buffer.FirstElement = 0;
    tileUAVDesc.Buffer.NumElements = TILE_COUNT * SHADER_ENTITY_TILE_BUCKET_COUNT;

    hr = Graphics->Device->CreateUnorderedAccessView(TileUAVBuffer, &tileUAVDesc, &TileUAV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Tile UAV!"));
    }
}

void FTileLightCullingPass::CreateBuffers()
{
    // 3. Debug heatmap 텍스처 + UAV (디버깅용)
    D3D11_TEXTURE2D_DESC heatMapDesc = {};
    heatMapDesc.Width = Graphics->screenWidth;
    heatMapDesc.Height = Graphics->screenHeight;
    heatMapDesc.MipLevels = 1;
    heatMapDesc.ArraySize = 1;
    heatMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    heatMapDesc.SampleDesc.Count = 1;
    heatMapDesc.Usage = D3D11_USAGE_DEFAULT;
    heatMapDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = Graphics->Device->CreateTexture2D(&heatMapDesc, nullptr, &DebugHeatmapTexture);

    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Heatmap Texture!"));
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC debugUAVDesc = {};
    debugUAVDesc.Format = heatMapDesc.Format;
    debugUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    hr = Graphics->Device->CreateUnorderedAccessView(DebugHeatmapTexture, &debugUAVDesc, &DebugHeatmapUAV);

    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Heatmap UAV!"));
    }

    //// 4. TileLight Culling 설정용 ConstantBuffer (2.5D 켜기/끄기 등)
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.ByteWidth = (sizeof(TileLightCullSettings) + 0xf) & 0xfffffff0; // struct 정의 필요
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = Graphics->Device->CreateBuffer(&cbDesc, nullptr, &TileLightConstantBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create TileLight Constant Buffer!"));
    }
}

void FTileLightCullingPass::Release()
{
    SAFE_RELEASE(TileUAVBuffer);
    SAFE_RELEASE(TileUAV);
    SAFE_RELEASE(DebugHeatmapTexture);
    SAFE_RELEASE(DebugHeatmapUAV);
    SAFE_RELEASE(TileLightConstantBuffer);

}

void FTileLightCullingPass::Dispatch()
{
    // 스레드 그룹 수 계산
    const UINT groupSizeX = (Graphics->screenWidth + TILE_SIZE - 1) / TILE_SIZE;
    const UINT groupSizeY = (Graphics->screenHeight + TILE_SIZE - 1) / TILE_SIZE;

    // 1. Constant Buffer 업데이트
    TileLightCullSettings settings = {};
    settings.ScreenSize[0] = Graphics->screenWidth;
    settings.ScreenSize[1] = Graphics->screenHeight;
    settings.Enable25DCulling = 1;                      // TODO : IMGUI 연결!

    Graphics->DeviceContext->UpdateSubresource(TileLightConstantBuffer, 0, nullptr, &settings, 0, 0);
    Graphics->DeviceContext->CSSetConstantBuffers(0, 1, &TileLightConstantBuffer);

    // 2. UAV 바인딩
    Graphics->DeviceContext->CSSetUnorderedAccessViews(0, 1, &TileUAV, nullptr);
    Graphics->DeviceContext->CSSetUnorderedAccessViews(3, 1, &DebugHeatmapUAV, nullptr); // register(u3)

    // 3. 셰이더 바인딩
    Graphics->DeviceContext->CSSetShader(ComputeShader, nullptr, 0);

    // 4. 디스패치
    Graphics->DeviceContext->Dispatch(groupSizeX, groupSizeY, 1);

    // 5. 바인딩 해제
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    Graphics->DeviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    Graphics->DeviceContext->CSSetUnorderedAccessViews(3, 1, &nullUAV, nullptr);
}

void FTileLightCullingPass::ClearUAVs()
{
    // UAV 초기화용 zero값
    UINT clearColor[4] = { 0, 0, 0, 0 };

    // 1. 타일 마스크 초기화
    Graphics->DeviceContext->ClearUnorderedAccessViewUint(TileUAV, clearColor);

    // 2. 히트맵 초기화
    float clearColorF[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->ClearUnorderedAccessViewFloat(DebugHeatmapUAV, clearColorF);
}
