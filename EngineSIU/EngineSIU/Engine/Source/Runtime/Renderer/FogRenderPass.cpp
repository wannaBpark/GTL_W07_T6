#include "FogRenderPass.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Define.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include <wchar.h>

// 정점 구조체 (전체 화면 Quad용)
struct Vertex
{
    float Position[3];
    float TexCoord[2];
};

FFogRenderPass::FFogRenderPass()
    : Graphics(nullptr)
    , ShaderManager(nullptr)
    , SpriteVertexBuffer(nullptr)
    , SpriteIndexBuffer(nullptr)
    , SpriteVertexShader(nullptr)
    , SpritePixelShader(nullptr)
    , InputLayout(nullptr)
{
}

FFogRenderPass::~FFogRenderPass()
{
    if (SpriteVertexBuffer) { SpriteVertexBuffer->Release(); SpriteVertexBuffer = nullptr; }
    if (SpriteIndexBuffer) { SpriteIndexBuffer->Release();  SpriteIndexBuffer = nullptr; }
    if (SpriteVertexShader) { SpriteVertexShader->Release(); SpriteVertexShader = nullptr; }
    if (SpritePixelShader) { SpritePixelShader->Release(); SpritePixelShader = nullptr; }
    if (InputLayout) { InputLayout->Release(); InputLayout = nullptr; }
}

void FFogRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;
    ShaderManager = InShaderManager;
    CreateSpriteResources();
    CreateShader();
}

void FFogRenderPass::CreateSpriteResources()
{
    // 전체 화면 Quad 정점 데이터 (정규화 좌표 사용)
    Vertex vertices[] =
    {
        { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
    };

    // 인덱스 데이터 (두 삼각형으로 Quad 구성)
    unsigned short indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    // 정점 버퍼 생성
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    HRESULT hr = Graphics->Device->CreateBuffer(&vbDesc, &vbData, &SpriteVertexBuffer);


    // 인덱스 버퍼 생성
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    hr = Graphics->Device->CreateBuffer(&ibDesc, &ibData, &SpriteIndexBuffer);
}

void FFogRenderPass::CreateShader()
{
    // 입력 레이아웃 정의: POSITION과 TEXCOORD
    D3D11_INPUT_ELEMENT_DESC fogInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // 정점 셰이더 및 입력 레이아웃 생성
    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"FogVertexShader",
        L"Shaders/FogVertexShader.hlsl",
        "mainVS",
        fogInputLayout,
        ARRAYSIZE(fogInputLayout)
    );
    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(
        L"FogPixelShader",
        L"Shaders/FogPixelShader.hlsl",
        "mainPS"
    );

    // 생성된 셰이더와 입력 레이아웃 획득
    SpriteVertexShader = ShaderManager->GetVertexShaderByKey(L"FogVertexShader");
    SpritePixelShader = ShaderManager->GetPixelShaderByKey(L"FogPixelShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"FogVertexShader");

    CreateSceneSrv();
}

void FFogRenderPass::CreateSceneSrv()
{
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&sampDesc, &Sampler);
}

void FFogRenderPass::PrepareRenderState(ID3D11ShaderResourceView* DepthSRV)
{
    // 셰이더 설정
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, nullptr);

    Graphics->DeviceContext->VSSetShader(SpriteVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(SpritePixelShader, nullptr, 0);

    // SRV & Sampler 바인딩
    Graphics->DeviceContext->PSSetShaderResources(126, 1, &SceneSRV);
    Graphics->DeviceContext->PSSetShaderResources(127, 1, &DepthSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Sampler);
}

void FFogRenderPass::RenderFog(const std::shared_ptr<FEditorViewportClient>& ActiveViewport, ID3D11ShaderResourceView* DepthSRV, AActor* Fog)
{
    D3D11_VIEWPORT vp = ActiveViewport->GetD3DViewport();
    UpdateSceneSRV();

    UpdateScreenConstant(vp);

    PrepareRenderState(DepthSRV);

    UpdateFogConstant(ActiveViewport, Cast<UHeightFogComponent>(Fog->GetRootComponent()));

    UINT stride = sizeof(Vertex), offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &SpriteVertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(SpriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    Graphics->DeviceContext->DrawIndexed(6, 0, 0);
}

void FFogRenderPass::UpdateSceneSRV()
{
    // 화면 크기가 변경되었으면 SRV를 재생성
    if (screenWidth != Graphics->screenWidth || screenHeight != Graphics->screenHeight) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        // 기존 SRV가 있다면 해제
        if (SceneSRV) { SceneSRV->Release(); SceneSRV = nullptr; }

        HRESULT hr = Graphics->Device->CreateShaderResourceView(Graphics->SceneColorBuffer, &srvDesc, &SceneSRV);
        if (FAILED(hr)) {
            return;
        }
        screenWidth = Graphics->screenWidth;
        screenHeight = Graphics->screenHeight;
    }
}

void FFogRenderPass::UpdateScreenConstant(const D3D11_VIEWPORT& viewport)
{
    float sw = float(screenWidth);
    float sh = float(screenHeight);

    FScreenConstants sc;
    sc.ScreenSize = { sw, sh };
    sc.UVOffset = { viewport.TopLeftX / sw, viewport.TopLeftY / sh };
    sc.UVScale = { viewport.Width / sw, viewport.Height / sh };
    sc.Padding = { 0.0f, 0.0f };

    BufferManager->UpdateConstantBuffer(TEXT("FScreenConstants"), sc);
    BufferManager->BindConstantBuffer(TEXT("FScreenConstants"), 0, EShaderStage::Pixel);
}

void FFogRenderPass::UpdateFogConstant(const std::shared_ptr<FEditorViewportClient>& ActiveViewport, UHeightFogComponent* Fog)
{
    FMatrix View = ActiveViewport->View;
    FMatrix Projection = ActiveViewport->GetProjectionMatrix();
    FMatrix ViewProj = View * Projection;
    FMatrix Inverse = FMatrix::Inverse(ViewProj);
    FFogConstants Constants; 
    {
        Constants.InvViewProj = Inverse;
        Constants.FogColor = Fog->GetFogColor();
        Constants.CameraPos = ActiveViewport->ViewTransformPerspective.GetLocation();
        Constants.FogDensity = Fog->GetFogDensity();
        Constants.FogHeightFalloff = Fog->GetFogHeightFalloff();
        Constants.StartDistance = Fog->GetStartDistance();
        Constants.FogCutoffDistance = Fog->GetFogCutoffDistance();
        Constants.FogMaxOpacity = Fog->GetFogMaxOpacity();
        if (ActiveViewport->IsOrtho()) {
            Constants.FogMaxOpacity = 0;
        }
        Constants.FogPosition = Fog->GetWorldLocation();
    }
    //상수버퍼 업데이트
    BufferManager->UpdateConstantBuffer(TEXT("FFogConstants"), Constants);
    //상수버퍼 바인딩
    ID3D11Buffer* FogConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FFogConstants"));
    BufferManager->BindConstantBuffer(TEXT("FFogConstants"), 1, EShaderStage::Pixel);
}
