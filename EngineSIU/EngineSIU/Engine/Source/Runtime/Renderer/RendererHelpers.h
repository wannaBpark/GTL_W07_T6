#pragma once
#include "Launch/EngineLoop.h"

enum class EShaderSRVSlot : int8
{
    SRV_SceneDepth = 99,
    SRV_Scene = 100,
    SRV_PostProcess = 101,
    SRV_EditorOverlay = 102,
    SRV_Depth = 103,
    SRV_Fog = 104,
    
    SRV_Viewport = 120,

    SRV_MAX = 127,
};

namespace MaterialUtils
{
    inline void UpdateMaterial(FDXDBufferManager* BufferManager, FGraphicsDevice* Graphics, const FObjMaterialInfo& MaterialInfo)
    {
        FMaterialConstants data;
        data.DiffuseColor = MaterialInfo.Diffuse;
        data.TransparencyScalar = MaterialInfo.TransparencyScalar;
        
        data.SpecularColor = MaterialInfo.Specular;
        data.SpecularScalar = MaterialInfo.SpecularScalar;
        
        data.EmissiveColor = MaterialInfo.Emissive;
        data.DensityScalar = MaterialInfo.DensityScalar;
        
        data.AmbientColor = MaterialInfo.Ambient;

        BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), data);

        if (MaterialInfo.bHasTexture)
        {
            std::shared_ptr<FTexture> Texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.DiffuseTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &Texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &Texture->SamplerState);
        }
        else
        {
            ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
            ID3D11SamplerState* NullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(0, 1, NullSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, NullSampler);
        }
    }
}
