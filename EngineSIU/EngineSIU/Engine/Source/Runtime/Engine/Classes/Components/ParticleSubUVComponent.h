#pragma once
#define _TCHAR_DEFINED
#include <wrl.h>
#include "UBillboardComponent.h"

// ParticleSubUVComponent: 서브UV 파티클 컴포넌트 (Billboard 컴포넌트를 상속)
class UParticleSubUVComponent : public UBillboardComponent
{
    DECLARE_CLASS(UParticleSubUVComponent, UBillboardComponent)

public:
    UParticleSubUVComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    // 텍스처 아틀라스의 셀 수 설정 (행, 열)
    void SetRowColumnCount(int cellsPerRow, int cellsPerColumn);
  
    virtual void SetTexture(const FWString& _fileName) override;
protected:
    // 애니메이션 반복 여부 (Loop)
    bool bIsLoop = true;

    // 현재 애니메이션 프레임 (열, 행 인덱스)
    int indexU = 0;
    int indexV = 0;

    // 누적 시간 (프레임 전환을 위한)
    float elapsedTime = 0.0f;
    // 프레임 당 지속 시간 (밀리초 단위, 필요에 따라 조정)
    float FrameDuration = 75.0f;

    // 텍스처 아틀라스의 셀 수 (행, 열)
    int CellsPerRow = 1;
    int CellsPerColumn = 1;

    // 버텍스 데이터(쿼드)를 생성하여 BufferManager를 통해 버퍼를 생성 또는 업데이트
    void CreateOrUpdateVertexBuffer();
};
