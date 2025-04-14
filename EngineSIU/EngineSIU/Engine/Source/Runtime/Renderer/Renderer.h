#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include <d3dcompiler.h>

#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Set.h"

#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDBufferManager.h"


enum class EResourceType : uint8;
class FSceneRenderPass;
class UWorld;
class UObject;

class FDXDShaderManager;
class FEditorViewportClient;

class FRenderTargetRHI;

class FStaticMeshRenderPass;
class FBillboardRenderPass;
class FGizmoRenderPass;
class FUpdateLightBufferPass;
class FDepthBufferDebugPass;
class FWorldNormalDebugPass;
class FLineRenderPass;
class FFogRenderPass;
class FCompositingPass;
class FSlateRenderPass;

class FRenderer
{
public:
    //==========================================================================
    // 초기화/해제 관련 함수
    //==========================================================================
    void Initialize(FGraphicsDevice* graphics, FDXDBufferManager* bufferManager);
    void Release();

    //==========================================================================
    // 렌더 패스 관련 함수
    //==========================================================================
    void PrepareRender();
    void ClearRenderArr();
    void SetRenderResource(EResourceType Type, FRenderTargetRHI* RenderTargetRHI, bool bClear = true, bool bIncludeDSV = true);
    void Render(const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderViewport(const std::shared_ptr<FEditorViewportClient>& Viewport); // TODO: 추후 RenderSlate로 변경해야함

    //==========================================================================
    // 버퍼 생성/해제 함수 (템플릿 포함)
    //==========================================================================
public:
    template<typename T>
    ID3D11Buffer* CreateImmutableVertexBuffer(const FString& key, const TArray<T>& Vertices);

    ID3D11Buffer* CreateImmutableIndexBuffer(const FString& key, const TArray<uint32>& indices);
    
    // 상수 버퍼 생성/해제
    void CreateConstantBuffers();
    void ReleaseConstantBuffer();

public:
    FGraphicsDevice* Graphics;
    FDXDBufferManager* BufferManager;
    FDXDShaderManager* ShaderManager = nullptr;

    FStaticMeshRenderPass* StaticMeshRenderPass = nullptr;
    FBillboardRenderPass* BillboardRenderPass = nullptr;
    FGizmoRenderPass* GizmoRenderPass = nullptr;
    FUpdateLightBufferPass* UpdateLightBufferPass = nullptr;
    FLineRenderPass* LineRenderPass = nullptr;
    FFogRenderPass* FogRenderPass = nullptr;
    
    FCompositingPass* CompositingPass = nullptr;
    
    FSlateRenderPass* SlateRenderPass = nullptr;
};

template<typename T>
inline ID3D11Buffer* FRenderer::CreateImmutableVertexBuffer(const FString& key, const TArray<T>& Vertices)
{
    FVertexInfo VertexBufferInfo;
    BufferManager->CreateVertexBuffer(key, Vertices, VertexBufferInfo);
    return VertexBufferInfo.VertexBuffer;
}

inline ID3D11Buffer* FRenderer::CreateImmutableIndexBuffer(const FString& key, const TArray<uint32>& indices)
{
    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer(key, indices, IndexInfo);
    return IndexInfo.IndexBuffer;
}
