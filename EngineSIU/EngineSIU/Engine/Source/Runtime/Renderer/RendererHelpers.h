#pragma once
#include "Launch/EngineLoop.h"

namespace RendererHelpers {
    
    inline FMatrix CalculateMVP(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection) {
        return Model * View * Projection;
    }

    inline FMatrix CalculateNormalMatrix(const FMatrix& Model) {
        return FMatrix::Transpose(FMatrix::Inverse(Model));
    }
}

namespace MaterialUtils {
    inline void UpdateMaterial(FDXDBufferManager* BufferManager, FGraphicsDevice* Graphics, const FObjMaterialInfo& MaterialInfo) {
        FMaterialConstants data;
        data.DiffuseColor = MaterialInfo.Diffuse;
        data.TransparencyScalar = MaterialInfo.TransparencyScalar;
        data.AmbientColor = MaterialInfo.Ambient;
        data.DensityScalar = MaterialInfo.DensityScalar;
        data.SpecularColor = MaterialInfo.Specular;
        data.SpecularScalar = MaterialInfo.SpecularScalar;
        data.EmmisiveColor = MaterialInfo.Emissive;

        BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), data);
        // Begin Test
        BufferManager->UpdateConstantBuffer(TEXT("FTextureFlagConstants"), MaterialInfo.TextureFlags);
        // End Test
        // bHasDiffuseTexture
        if (MaterialInfo.TextureFlags & 1 << 1) {
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.DiffuseTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        else
        {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);

        }
        // bHasNormalTexture
        if (MaterialInfo.TextureFlags & 1 << 2)
        {
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.BumpTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(1, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        else {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(1, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
        }
    }
}
