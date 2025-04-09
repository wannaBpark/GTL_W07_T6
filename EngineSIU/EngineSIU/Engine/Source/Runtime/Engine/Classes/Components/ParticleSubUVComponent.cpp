#include "ParticleSubUVComponent.h"
#include "EngineLoop.h"
#include "UObject/Casts.h"
#include "D3D11RHI/DXDBufferManager.h"

// 생성자: 기본 값 설정
UParticleSubUVComponent::UParticleSubUVComponent()
{
    SetType(StaticClass()->GetName());
    bIsLoop = true;
}

// Duplicate: 버퍼 포인터는 복사하지 않고 애니메이션 상태만 복제
UObject* UParticleSubUVComponent::Duplicate(UObject* InOuter)
{
    UParticleSubUVComponent* NewComponent = Cast<UParticleSubUVComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->bIsLoop = bIsLoop;
        NewComponent->CellsPerRow = CellsPerRow;
        NewComponent->CellsPerColumn = CellsPerColumn;
        NewComponent->indexU = indexU;
        NewComponent->indexV = indexV;
        NewComponent->elapsedTime = elapsedTime;
        NewComponent->UVScale = UVScale;
        NewComponent->UVOffset = UVOffset;
    }
    return NewComponent;
}

// InitializeComponent: 초기화 시 버텍스 버퍼 생성
void UParticleSubUVComponent::InitializeComponent()
{

}

// TickComponent: 매 프레임 애니메이션 업데이트 및 UV 오프셋 계산
void UParticleSubUVComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    if (!IsActive() || !Texture)
        return;

    // 텍스처 크기를 기반으로 셀 크기 계산 (행, 열)
    uint32 CellWidth = Texture->Width / CellsPerColumn;
    uint32 CellHeight = Texture->Height / CellsPerRow;

    float UVSaleX = 1.0f / static_cast<float>(CellsPerColumn);
    float UVSaleY = 1.0f / static_cast<float>(CellsPerRow);

    // 시간 누적 후 프레임 전환
    elapsedTime += (DeltaTime * 1000);
    if (elapsedTime >= FrameDuration)
    {
        indexU++;
        elapsedTime = 0.0f;
    }
    if (indexU >= CellsPerColumn)
    {
        indexU = 0;
        indexV++;
    }
    if (indexV >= CellsPerRow)
    {
        indexU = 0;
        indexV = 0;
        if (!bIsLoop)
        {
            Deactivate();
        }
    }

    float UVOffsetX = static_cast<float>(indexU) * UVSaleX;
    float UVOffsetY = static_cast<float>(indexV) * UVSaleY;

    UVScale = FVector2D(UVSaleX, UVSaleY);
    UVOffset = FVector2D(UVOffsetX, UVOffsetY);
}

// SetRowColumnCount: 셀 수 변경 시 버텍스 버퍼 재생성
void UParticleSubUVComponent::SetRowColumnCount(int cellsPerRow, int cellsPerColumn)
{
    CellsPerRow = cellsPerRow;
    CellsPerColumn = cellsPerColumn;
}

void UParticleSubUVComponent::SetTexture(const FWString& _fileName)
{
    Texture = FEngineLoop::ResourceManager.GetTexture(_fileName);
    std::string str(_fileName.begin(), _fileName.end());
    BufferKey = FString(str);

}