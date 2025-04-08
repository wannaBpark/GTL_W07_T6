#include "UTextComponent.h"

#include "World/World.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/Casts.h"

UTextComponent::UTextComponent()
{
    SetType(StaticClass()->GetName());
}

UObject* UTextComponent::Duplicate()
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate());

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
    vertexTextureArr.Empty();
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
    for (int i = 0; i < vertexTextureArr.Num(); i++)
    {
        Quad.Add(FVector(vertexTextureArr[i].x,
            vertexTextureArr[i].y, vertexTextureArr[i].z));
    }

    return CheckPickingOnNDC(Quad, pfNearHitDistance);
}

void UTextComponent::SetText(const FWString& text)
{
    Text = text;
    if (Text.empty())
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text is empty");

        vertexTextureArr.Empty();
        Quad.Empty();

        return;
    }
    int textSize = static_cast<int>(Text.size());


    uint32 BitmapWidth = Texture->Width;
    uint32 BitmapHeight = Texture->Height;

    float CellWidth = float(BitmapWidth) / ColumnCount;
    float CellHeight = float(BitmapHeight) / RowCount;

    float nTexelUOffset = CellWidth / BitmapWidth;
    float nTexelVOffset = CellHeight / BitmapHeight;

    for (int i = 0; i < Text.size(); i++)
    {
        FVertexTexture leftUP = { -1.0f,1.0f,0.0f,0.0f,0.0f };
        FVertexTexture rightUP = { 1.0f,1.0f,0.0f,1.0f,0.0f };
        FVertexTexture leftDown = { -1.0f,-1.0f,0.0f,0.0f,1.0f };
        FVertexTexture rightDown = { 1.0f,-1.0f,0.0f,1.0f,1.0f };
        rightUP.u *= nTexelUOffset;
        leftDown.v *= nTexelVOffset;
        rightDown.u *= nTexelUOffset;
        rightDown.v *= nTexelVOffset;

        leftUP.x += QuadWidth * i;
        rightUP.x += QuadWidth * i;
        leftDown.x += QuadWidth * i;
        rightDown.x += QuadWidth * i;

        float startU = 0.0f;
        float startV = 0.0f;

        SetStartUV(Text[i], startU, startV);
        leftUP.u += (nTexelUOffset * startU);
        leftUP.v += (nTexelVOffset * startV);
        rightUP.u += (nTexelUOffset * startU);
        rightUP.v += (nTexelVOffset * startV);
        leftDown.u += (nTexelUOffset * startU);
        leftDown.v += (nTexelVOffset * startV);
        rightDown.u += (nTexelUOffset * startU);
        rightDown.v += (nTexelVOffset * startV);

        vertexTextureArr.Add(leftUP);
        vertexTextureArr.Add(rightUP);
        vertexTextureArr.Add(leftDown);
        vertexTextureArr.Add(rightUP);
        vertexTextureArr.Add(rightDown);
        vertexTextureArr.Add(leftDown);
    }

    float lastX = -1.0f + QuadWidth * Text.size();
    Quad.Add(FVector(-1.0f, 1.0f, 0.0f));
    Quad.Add(FVector(-1.0f, -1.0f, 0.0f));
    Quad.Add(FVector(lastX, 1.0f, 0.0f));
    Quad.Add(FVector(lastX, -1.0f, 0.0f));
    std::string key;

    key.assign(text.begin(), text.end());
    
    TextAtlasBufferKey = FString(key);

    CreateTextTextureVertexBuffer(vertexTextureArr);
}
void UTextComponent::SetStartUV(wchar_t hangul, float& outStartU, float& outStartV)
{
    //대문자만 받는중
    int StartU = 0;
    int StartV = 0;
    int offset = -1;

    if (hangul == L' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (hangul >= L'A' && hangul <= L'Z') {

        StartU = 11;
        StartV = 0;
        offset = hangul - L'A'; // 대문자 위치
    }
    else if (hangul >= L'a' && hangul <= L'z') {
        StartU = 37;
        StartV = 0;
        offset = (hangul - L'a'); // 소문자는 대문자 다음 위치
    }
    else if (hangul >= L'0' && hangul <= L'9') {
        StartU = 1;
        StartV = 0;
        offset = (hangul - L'0'); // 숫자는 소문자 다음 위치
    }
    else if (hangul >= L'가' && hangul <= L'힣')
    {
        StartU = 63;
        StartV = 0;
        offset = hangul - L'가'; // 대문자 위치
    }

    if (offset == -1)
    {
        UE_LOG(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}
void UTextComponent::SetStartUV(char alphabet, float& outStartU, float& outStartV)
{
    //대문자만 받는중
    int StartU = 0;
    int StartV = 0;
    int offset = -1;


    if (alphabet == ' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (alphabet >= 'A' && alphabet <= 'Z') {

        StartU = 1;
        StartV = 4;
        offset = alphabet - 'A'; // 대문자 위치
    }
    else if (alphabet >= 'a' && alphabet <= 'z') {
        StartU = 1;
        StartV = 6;
        offset = (alphabet - 'a'); // 소문자는 대문자 다음 위치
    }
    else if (alphabet >= '0' && alphabet <= '9') {
        StartU = 0;
        StartV = 3;
        offset = (alphabet - '0'); // 숫자는 소문자 다음 위치
    }

    if (offset == -1)
    {
       UE_LOG(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);

}
void UTextComponent::CreateTextTextureVertexBuffer(const TArray<FVertexTexture>& Vertex)
{
    FEngineLoop::Renderer.CreateImmutableVertexBuffer(TextAtlasBufferKey, Vertex);
}
