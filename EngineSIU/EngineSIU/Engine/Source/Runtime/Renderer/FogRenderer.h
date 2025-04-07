#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Set.h"
#include <D3D11RHI/GraphicDevice.h>
#include <Math/Color.h>

struct alignas(16) FFogConstants
{
    FMatrix InvViewProj;
    FLinearColor FogColor;
    FVector CameraPos;
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
};
class FFogRenderer
{
public:
    void Initialize(FGraphicsDevice* device);
    void UpdateConstant(FFogConstants Constant);
    void Release();
    ID3D11VertexShader* GetVertexShader() { return VertexShader; }
    ID3D11PixelShader* GetPixelShader() { return PixelShader; }
    ID3D11Buffer* GetVertexBuffer() { return FogVertexBuffer; }
    ID3D11Buffer* GetIndexBuffer() { return FogIndexBuffer; }
    ID3D11InputLayout* GetInputLayout() { return InputLayout; }
    ID3D11Buffer* GetConstantBuffer() { return FogConstantBuffer; }
    uint32 GetStride() { return Stride; }
    ID3D11SamplerState* GetSamplerState() { return FogSampler; }

private:
    void CreateShader();
    void CreateConstantBuffer();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateSamplerState();

private:
    FGraphicsDevice* Graphics = nullptr;
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11Buffer* FogVertexBuffer = nullptr;
    ID3D11Buffer* FogIndexBuffer = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;
    ID3D11Buffer* FogConstantBuffer = nullptr;
    ID3D11SamplerState* FogSampler = nullptr;

    uint32 Stride;
};