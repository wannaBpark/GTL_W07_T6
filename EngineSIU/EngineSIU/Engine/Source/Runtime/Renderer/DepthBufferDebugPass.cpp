#include "DepthBufferDebugPass.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include <wchar.h>

// 정점 구조체 (전체 화면 Quad용)
struct Vertex
{
    float Position[3];
    float TexCoord[2];
};

FDepthBufferDebugPass::FDepthBufferDebugPass()
    : Graphics(nullptr)
    , ShaderManager(nullptr)
    , SpriteVertexBuffer(nullptr)
    , SpriteIndexBuffer(nullptr)
    , SpriteVertexShader(nullptr)
    , DepthBufferPixelShader(nullptr)
    , InputLayout(nullptr)
    , DepthBufferSRV(nullptr)
    , DepthStateDisable(nullptr)
{
}

FDepthBufferDebugPass::~FDepthBufferDebugPass()
{
    if (SpriteVertexBuffer) { SpriteVertexBuffer->Release(); SpriteVertexBuffer = nullptr; }
    if (SpriteIndexBuffer) { SpriteIndexBuffer->Release();  SpriteIndexBuffer = nullptr; }
    if (SpriteVertexShader) { SpriteVertexShader->Release(); SpriteVertexShader = nullptr; }
    if (DepthBufferPixelShader) { DepthBufferPixelShader->Release(); DepthBufferPixelShader = nullptr; }
    if (InputLayout) { InputLayout->Release(); InputLayout = nullptr; }
    if (DepthStateDisable) { DepthStateDisable->Release(); DepthStateDisable = nullptr; }
    // DepthBufferSRV는 외부에서 관리하므로 해제하지 않음.
}

void FDepthBufferDebugPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;
    ShaderManager = InShaderManager;
    CreateSpriteResources();
    CreateShader();
}

void FDepthBufferDebugPass::CreateSpriteResources()
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

    // Depth Write를 비활성화한 DepthStencilState 생성 (Depth Buffer 렌더링용)
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    hr = Graphics->Device->CreateDepthStencilState(&dsDesc, &DepthStateDisable);

}

void FDepthBufferDebugPass::CreateShader()
{
    // 입력 레이아웃 정의: POSITION과 TEXCOORD
    D3D11_INPUT_ELEMENT_DESC depthInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // 정점 셰이더 및 입력 레이아웃 생성
    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"DepthBufferVertexShader",
        L"Shaders/DepthBufferVertexShader.hlsl",
        "mainVS",
        depthInputLayout,
        ARRAYSIZE(depthInputLayout)
    );
    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(
        L"DepthBufferPixelShader",
        L"Shaders/DepthBufferPixelShader.hlsl",
        "mainPS"
    );

    // 생성된 셰이더와 입력 레이아웃 획득
    SpriteVertexShader = ShaderManager->GetVertexShaderByKey(L"DepthBufferVertexShader");
    DepthBufferPixelShader = ShaderManager->GetPixelShaderByKey(L"DepthBufferPixelShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"DepthBufferVertexShader");

    CreateDepthBufferSrv();
}

void FDepthBufferDebugPass::CreateDepthBufferSrv()
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
    Graphics->Device->CreateSamplerState(&sampDesc, &DepthSampler);

}

void FDepthBufferDebugPass::PrepareRenderState()
{
    // 셰이더 설정
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, nullptr);
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    
    Graphics->DeviceContext->VSSetShader(SpriteVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(DepthBufferPixelShader, nullptr, 0);

    // SRV & Sampler 바인딩
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &DepthBufferSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &DepthSampler);


   
}

void FDepthBufferDebugPass::UpdateDepthBufferSRV()
{
    // 화면 크기가 변경되었으면 SRV를 재생성
    if (screenWidth != Graphics->screenWidth || screenHeight != Graphics->screenHeight) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        // 기존 SRV가 있다면 해제
        if (DepthBufferSRV) { DepthBufferSRV->Release(); DepthBufferSRV = nullptr; }

        HRESULT hr = Graphics->Device->CreateShaderResourceView(Graphics->DepthStencilBuffer, &srvDesc, &DepthBufferSRV);
        if (FAILED(hr)) {
            return;
        }
        screenWidth = Graphics->screenWidth;
        screenHeight = Graphics->screenHeight;

    }
}

void FDepthBufferDebugPass::UpdateScreenConstant(const D3D11_VIEWPORT& viewport)
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

void FDepthBufferDebugPass::RenderDepthBuffer(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    D3D11_VIEWPORT vp = ActiveViewport->GetD3DViewport();
    UpdateDepthBufferSRV();
    
    PrepareRenderState();

    UpdateScreenConstant(vp);
  
    UINT stride = sizeof(Vertex), offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &SpriteVertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(SpriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    Graphics->DeviceContext->DrawIndexed(6, 0, 0);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, Graphics->DepthStencilView);

}
