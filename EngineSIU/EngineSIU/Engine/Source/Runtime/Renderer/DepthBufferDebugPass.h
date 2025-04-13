#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <memory>
class FGraphicsDevice;
class FDXDShaderManager;
class FDXDBufferManager;
class FEditorViewportClient;

class FDepthBufferDebugPass
{
public:
    FDepthBufferDebugPass();
    ~FDepthBufferDebugPass();

    // 초기화: 그래픽 디바이스와 셰이더 매니저를 등록
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager);

    // Depth Buffer 렌더링용 셰이더 생성 및 입력 레이아웃 설정
    void CreateShader();
  
    void PrepareRenderState();
    
    // Depth Buffer를 화면에 렌더링
    void Render(const std::shared_ptr<FEditorViewportClient>& Viewport);

    void UpdateScreenConstant(const D3D11_VIEWPORT& viewport);

private:
    FGraphicsDevice* Graphics;
    FDXDBufferManager* BufferManager;
    FDXDShaderManager* ShaderManager;

    // Depth Buffer 렌더링용 셰이더 및 입력 레이아웃
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    ID3D11SamplerState* DepthSampler;

    float screenWidth = 0;
    float screenHeight = 0;
};
