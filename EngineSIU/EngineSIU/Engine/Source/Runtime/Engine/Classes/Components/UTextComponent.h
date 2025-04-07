#pragma once

#define _TCHAR_DEFINED
#include <wrl.h>

#include "UBillboardComponent.h"

class UText : public UBillboardComponent
{
    DECLARE_CLASS(UText, UBillboardComponent)

public:
    UText();

    virtual UObject* Duplicate() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    void ClearText();
    void SetText(const FWString& _text);
    FWString GetText() { return text; }
    void SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn);
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexTextBuffer;
    TArray<FVertexTexture> vertexTextureArr;
    UINT numTextVertices;
protected:
    FWString text;

    TArray<FVector> quad;

    int quadSize = 2;

    int RowCount;
    int ColumnCount;

    float quadWidth = 2.0f;
    float quadHeight = 2.0f;

    void setStartUV(char alphabet, float& outStartU, float& outStartV);
    void setStartUV(wchar_t hangul, float& outStartU, float& outStartV);
    void CreateTextTextureVertexBuffer(const TArray<FVertexTexture>& _vertex, UINT byteWidth);

    void TextMVPRendering();
};
