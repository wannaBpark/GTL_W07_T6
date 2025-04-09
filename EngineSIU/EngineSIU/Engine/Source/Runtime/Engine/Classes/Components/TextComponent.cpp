#include "TextComponent.h"

#include "World/World.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/Casts.h"

UTextComponent::UTextComponent()
{
    SetType(StaticClass()->GetName());
}

UObject* UTextComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->TextAtlasBufferKey = TextAtlasBufferKey;
    NewComponent->Text = Text;
    NewComponent->Quad = Quad;
    NewComponent->QuadSize = QuadSize;
    NewComponent->RowCount = RowCount;
    NewComponent->ColumnCount = ColumnCount;
    NewComponent->QuadWidth = QuadWidth;
    NewComponent->QuadHeight = QuadHeight;

    return NewComponent;
}

void UTextComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UTextComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UTextComponent::ClearText()
{
   // vertexTextureArr.Empty();
}

void UTextComponent::SetText(const FWString& text)
{
    Text = text;
  
}

void UTextComponent::SetRowColumnCount(int cellsPerRow, int cellsPerColumn)
{
    RowCount = cellsPerRow;
    ColumnCount = cellsPerColumn;
}

int UTextComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    if (!(ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))) {
        return 0;
    }
    //TODO: quadWidth 고정으로 font사이즈 변경시 문제 발생할 수 있음
    const float quadWidth = 2.0f;
    float totalTextWidth = quadWidth * Text.size();
    float centerOffset = totalTextWidth / 2.0f;

    for (int i = 0; i < Text.size(); i++)
    {
        float offsetX = quadWidth * i - centerOffset;
        TArray<FVector> LetterQuad;
        LetterQuad.Add(FVector(-1.0f + offsetX, 1.0f, 0.0f));
        LetterQuad.Add(FVector(1.0f + offsetX, 1.0f, 0.0f));
        LetterQuad.Add(FVector(1.0f + offsetX, -1.0f, 0.0f));
        LetterQuad.Add(FVector(-1.0f + offsetX, -1.0f, 0.0f));

        float hitDistance = 0.0f;
        if (CheckPickingOnNDC(LetterQuad, hitDistance))
        {
            pfNearHitDistance = hitDistance;
            return 1;
        }
    }

    return 0;
}
