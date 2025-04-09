#pragma once

#define _TCHAR_DEFINED
#include <wrl.h>

#include "BillboardComponent.h"

class UTextComponent : public UBillboardComponent
{
    DECLARE_CLASS(UTextComponent, UBillboardComponent)

public:
    UTextComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    
    virtual void TickComponent(float DeltaTime) override;
    
    void ClearText();
    
    void SetText(const FWString& text);
    
    FString GetBufferKey() { return TextAtlasBufferKey; }
    
    FWString GetText() { return Text; }
    
    void SetRowColumnCount(int cellsPerRow, int cellsPerColumn);

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

private:
    void SetStartUV(char alphabet, float& outStartU, float& outStartV);
    void SetStartUV(wchar_t hangul, float& outStartU, float& outStartV);

protected:

    FWString Text;

    TArray<FVector> Quad;

    TArray<FVertexTexture> vertexTextureArr;

    int QuadSize = 2;

    int RowCount;
    int ColumnCount;

    float QuadWidth = 2.0f;
    float QuadHeight = 2.0f;
private:
    FString TextAtlasBufferKey;
};
