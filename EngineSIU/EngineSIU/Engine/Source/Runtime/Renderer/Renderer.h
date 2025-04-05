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

class FDXDShaderManager;

class ULightComponentBase;

class UWorld;

class UObject;

class FEditorViewportClient;

class UBillboardComponent;

class UGizmoBaseComponent;

class FStaticMeshRenderPass;

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
    void Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);

    // 개별 Render Pass
    //void RenderStaticMeshes(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);

    void RenderGizmoComponent(UGizmoBaseComponent* GizmoComp,
                              const std::shared_ptr<FEditorViewportClient>& ActiveViewport,
                              const UWorld* World);

    bool ShouldRenderGizmo(UGizmoBaseComponent* GizmoComp, const UWorld* World) const;

    void RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);

    void RenderBillboards(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);

    void RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);

    // 뷰 모드 변경
    void ChangeViewMode(EViewModeIndex evi) const;

    //==========================================================================
    // 셰이더 관련 함수
    //==========================================================================
    void CreateShader();
public:
    // Texture 셰이더
    void ReleaseTextureShader();
    void PrepareTextureShader() const;

    // Line 셰이더
    void ReleaseLineShader();
    void PrepareLineShader() const;
    void ProcessLineRendering(const FMatrix& View, const FMatrix& Projection);
    void DrawLineBatch(const FLinePrimitiveBatchArgs& linePrimitiveBatchArgs) const;

    //==========================================================================
    // 버퍼 생성/해제 함수 (템플릿 포함)
    //==========================================================================
public:
    template<typename T>
    ID3D11Buffer* CreateImmutableVertexBuffer(const FString& key, const TArray<T>& Vertices);
   
    ID3D11Buffer* CreateImmutableIndexBuffer(const FString& key, const TArray<uint32>& indices);
    
    // 상수 버퍼 생성/해제
    void CreateConstantBuffer();

    void ReleaseConstantBuffer();
    //==========================================================================
    // 업데이트 함수 (상수 버퍼, 재질 등)
    //==========================================================================
public:
    void UpdateLightBuffer() const;

    void UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const;

    void UpdateLitUnlitConstant(int isLit) const;

    void UpdateTextureConstant(float UOffset, float VOffset) const;

    void UpdateSubUVConstant(float _indexU, float _indexV) const;

    void PrepareSubUVConstant() const;

    void UpdatePerObjectConstant(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FVector4& UUIDColor, bool selected) const;
    //==========================================================================
    // 텍스처 버퍼 관련 함수
    //==========================================================================
public:
   void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11Buffer* pIndexBuffer, UINT numIndices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;
    void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;

public:
    FGraphicsDevice* Graphics;
    FDXDBufferManager* BufferManager;
    FDXDShaderManager* ShaderManager = nullptr;

    FStaticMeshRenderPass* StaticMeshRenderPass = nullptr;

    // Texture 셰이더 관련
    ID3D11VertexShader* VertexTextureShader = nullptr;
    ID3D11PixelShader* PixelBillBoardShader = nullptr;
    ID3D11InputLayout* BillBoardInputLayout = nullptr;
    uint32 TextureStride;
    
    struct FSubUVConstant
    {
        float indexU;
        float indexV;
    };

    // Line 셰이더 관련
    ID3D11VertexShader* VertexLineShader = nullptr;
    ID3D11PixelShader* PixelLineShader = nullptr;

private:
    TArray<UGizmoBaseComponent*> GizmoObjs;
    TArray<UBillboardComponent*> BillboardObjs;
    TArray<ULightComponentBase*> LightObjs;
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
