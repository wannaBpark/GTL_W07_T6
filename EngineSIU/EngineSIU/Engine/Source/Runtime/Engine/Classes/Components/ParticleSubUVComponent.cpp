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
UObject* UParticleSubUVComponent::Duplicate()
{
    UParticleSubUVComponent* NewComponent = Cast<UParticleSubUVComponent>(Super::Duplicate());
    if (NewComponent)
    {
        NewComponent->bIsLoop = bIsLoop;
        NewComponent->CellsPerRow = CellsPerRow;
        NewComponent->CellsPerColumn = CellsPerColumn;
        NewComponent->indexU = indexU;
        NewComponent->indexV = indexV;
        NewComponent->elapsedTime = elapsedTime;
        NewComponent->finalIndexU = finalIndexU;
        NewComponent->finalIndexV = finalIndexV;
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

    // 시간 누적 후 프레임 전환
    elapsedTime += DeltaTime;
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

    // 정규화된 오프셋 계산
    float normalWidthOffset = static_cast<float>(CellWidth) / Texture->Width;
    float normalHeightOffset = static_cast<float>(CellHeight) / Texture->Height;
    finalIndexU = static_cast<float>(indexU) * normalWidthOffset;
    finalIndexV = static_cast<float>(indexV) * normalHeightOffset;
    // UE_LOG(LogLevel::Error, "%f, %f", finalIndexU, finalIndexV);
     // 필요한 경우 동적으로 버텍스 버퍼를 업데이트하는 로직

}

// SetRowColumnCount: 셀 수 변경 시 버텍스 버퍼 재생성
void UParticleSubUVComponent::SetRowColumnCount(int cellsPerRow, int cellsPerColumn)
{
    CellsPerRow = cellsPerRow;
    CellsPerColumn = cellsPerColumn;
    CreateOrUpdateVertexBuffer();
}

void UParticleSubUVComponent::SetTexture(const FWString& _fileName)
{
    Texture = FEngineLoop::resourceMgr.GetTexture(_fileName);
    std::string str(_fileName.begin(), _fileName.end());
    BufferKey = FString(str);

}

// CreateOrUpdateVertexBuffer: 쿼드 버텍스 데이터를 생성하고, BufferManager를 통해 버텍스 버퍼 생성
void UParticleSubUVComponent::CreateOrUpdateVertexBuffer()
{
    if (!Texture)
    {
       UE_LOG(LogLevel::Warning, "UParticleSubUVComponent: Texture가 설정되지 않았습니다.");
        return;
    }

    // 셀 크기 계산
    uint32 CellWidth = Texture->Width / CellsPerColumn;
    uint32 CellHeight = Texture->Height / CellsPerRow;
    float normalWidthOffset = static_cast<float>(CellWidth) / Texture->Width;
    float normalHeightOffset = static_cast<float>(CellHeight) / Texture->Height;

    // 기본 쿼드 버텍스 데이터 (4개 정점)
    TArray<FVertexTexture> vertices = {
        {-1.0f,  1.0f, 0.0f, 0.0f, 0.0f},  // 좌상단
        { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f},  // 우상단
        {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},  // 좌하단
        { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f}   // 우하단
    };

    // UV 좌표 조정: 기본 쿼드에 대해 오른쪽과 아래쪽 좌표를 변경
    vertices[1].u = normalWidthOffset;
    vertices[2].v = normalHeightOffset;
    vertices[3].u = normalWidthOffset;
    vertices[3].v = normalHeightOffset;
    TArray<uint32> QuadTextureIndices = { 0, 1, 2, 1, 3, 2 };
    // BufferManager를 통해 고유 키(BufferKey)를 사용하여 Immutable 버텍스 버퍼 생성
    FEngineLoop::renderer.CreateImmutableVertexBuffer(BufferKey, vertices);
    FEngineLoop::renderer.CreateImmutableIndexBuffer(BufferKey, QuadTextureIndices);
}
