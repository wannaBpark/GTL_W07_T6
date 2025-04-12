#include "SLevelEditor.h"
#include <fstream>
#include <ostream>
#include <sstream>
#include "EngineLoop.h"
#include "UnrealClient.h"
#include "WindowsCursor.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "SlateCore/Widgets/SWindow.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;


SLevelEditor::SLevelEditor()
    : HSplitter(nullptr)
    , VSplitter(nullptr)
    , World(nullptr)
    , bMultiViewportMode(false)
{
}

void SLevelEditor::Initialize()
{
    EditorWidth = FEngineLoop::GraphicDevice.screenWidth;
    EditorHeight = FEngineLoop::GraphicDevice.screenHeight;
    
    for (size_t i = 0; i < 4; i++)
    {
        ViewportClients[i] = std::make_shared<FEditorViewportClient>();
        ViewportClients[i]->Initialize(i);
    }
    ActiveViewportClient = ViewportClients[0];
    VSplitter = new SSplitterV();
    VSplitter->Initialize(FRect(0.0f, EditorHeight * 0.5f - 10, EditorHeight, 20));
    VSplitter->OnDrag(FPoint(0, 0));
    HSplitter = new SSplitterH();
    HSplitter->Initialize(FRect(EditorWidth * 0.5f - 10, 0.0f, 20, EditorWidth));
    HSplitter->OnDrag(FPoint(0, 0));
    LoadConfig();

    SetEnableMultiViewport(bMultiViewportMode);
    ResizeLevelEditor();


    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
        {
        case EKeys::RightMouseButton:
        {
            // TODO: 지금 우클릭 하면 무조건 커서를 숨기게 되어있는데, Viewport에 Focus가 되어있을때만 숨겨지게 만들기
            // FWindowsCursor::SetMouseCursor(ECursorType::None);
            MousePinPosition = InMouseEvent.GetScreenSpacePosition();
            return;
        }

        default:
        {
            if (bMultiViewportMode)
            {
                // 마우스 이벤트가 일어난 위치의 뷰포트를 선택
                POINT Point;
                GetCursorPos(&Point);
                ScreenToClient(GEngineLoop.AppWnd, &Point);
                FVector2D ClientPos = FVector2D{static_cast<float>(Point.x), static_cast<float>(Point.y)};
                SelectViewport(ClientPos);
                VSplitter->OnPressed({ClientPos.X, ClientPos.Y});
                HSplitter->OnPressed({ClientPos.X, ClientPos.Y});
            }
            return;
        }
        }
    });

    Handler->OnMouseMoveDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        // 에디터 카메라 이동 로직
        if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
        {
            ActiveViewportClient->MouseMove(InMouseEvent);
            // TODO: 무한마우스 구현하기
            // FWindowsCursor::SetPosition(static_cast<int32>(MousePinPosition.X), static_cast<int32>(MousePinPosition.Y));
        }

        else if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
        {
            const auto& [DeltaX, DeltaY] = InMouseEvent.GetCursorDelta();
            if (VSplitter->IsPressing())
            {
                VSplitter->OnDrag(FPoint(DeltaX, DeltaY));
            }
            if (HSplitter->IsPressing())
            {
                HSplitter->OnDrag(FPoint(DeltaX, DeltaY));
            }
            if (VSplitter->IsPressing() || HSplitter->IsPressing())
            {
                FEngineLoop::GraphicDevice.OnResize(GEngineLoop.AppWnd);
                ResizeViewports();
            }
        }

        // 멀티 뷰포트일 때, 커서 변경 로직
        if (bMultiViewportMode)
        {
            POINT Point;
            GetCursorPos(&Point);
            ScreenToClient(GEngineLoop.AppWnd, &Point);
            FVector2D ClientPos = FVector2D{static_cast<float>(Point.x), static_cast<float>(Point.y)};
            const bool bIsVerticalHovered = VSplitter->IsHover({ClientPos.X, ClientPos.Y});
            const bool bIsHorizontalHovered = HSplitter->IsHover({ClientPos.X, ClientPos.Y});

            if (bIsHorizontalHovered && bIsVerticalHovered)
            {
                FWindowsCursor::SetMouseCursor(ECursorType::ResizeAll);
            }
            else if (bIsHorizontalHovered)
            {
                FWindowsCursor::SetMouseCursor(ECursorType::ResizeLeftRight);
            }
            else if (bIsVerticalHovered)
            {
                FWindowsCursor::SetMouseCursor(ECursorType::ResizeUpDown);
            }
            else
            {
                FWindowsCursor::SetMouseCursor(ECursorType::Arrow);
            }
        }
    });

    Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
        {
        case EKeys::RightMouseButton:
        {
            // FWindowsCursor::SetMouseCursor(ECursorType::Arrow);
            return;
        }

        // Viewport 선택 로직
        case EKeys::LeftMouseButton:
        {
            VSplitter->OnReleased();
            HSplitter->OnReleased();
            return;
        }

        default:
            return;
        }
    });

    Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        if (InKeyEvent.GetCharacter() == 'M')
        {
            FEngineLoop::GraphicDevice.OnResize(GEngineLoop.AppWnd);
            SetEnableMultiViewport(!IsMultiViewport());
        }
    });
}

void SLevelEditor::Tick(float DeltaTime)
{
    for (std::shared_ptr<FEditorViewportClient> Viewport : ViewportClients)
    {
        Viewport->Tick(DeltaTime);
    }
}

void SLevelEditor::Release()
{
    SaveConfig();
    delete VSplitter;
    delete HSplitter;
}

void SLevelEditor::SelectViewport(FVector2D InPoint)
{
    for (int Idx = 0; Idx < 4; Idx++)
    {
        if (ViewportClients[Idx]->IsSelected(InPoint))
        {
            SetViewportClient(Idx);
            break;
        }
    }
}

void SLevelEditor::ResizeLevelEditor()
{
    float PrevWidth = EditorWidth;
    float PrevHeight = EditorHeight;
    EditorWidth = FEngineLoop::GraphicDevice.screenWidth;
    EditorHeight = FEngineLoop::GraphicDevice.screenHeight;
    
    //HSplitter 에는 바뀐 width 비율이 들어감 
    HSplitter->OnResize(EditorWidth/PrevWidth, EditorHeight);
    //HSplitter 에는 바뀐 Height 비율이 들어감 
    VSplitter->OnResize(EditorWidth, EditorHeight/PrevHeight);
    ResizeViewports();
}

void SLevelEditor::ResizeViewports()
{
    if (bMultiViewportMode) {
        if (GetViewports()[0]) {
            for (int i = 0;i < 4;++i)
            {
                GetViewports()[i]->ResizeViewport(VSplitter->SideLT->Rect, VSplitter->SideRB->Rect,
                    HSplitter->SideLT->Rect, HSplitter->SideRB->Rect);
            }
        }
    }
    else
    {
        ActiveViewportClient->GetViewport()->ResizeViewport(FRect(0.0f, 0.0f, EditorWidth, EditorHeight));
    }
}

void SLevelEditor::SetEnableMultiViewport(bool bIsEnable)
{
    bMultiViewportMode = bIsEnable;
    ResizeViewports();
}

bool SLevelEditor::IsMultiViewport() const
{
    return bMultiViewportMode;
}

void SLevelEditor::LoadConfig()
{
    auto config = ReadIniFile(IniFilePath);
    ActiveViewportClient->Pivot.X = GetValueFromConfig(config, "OrthoPivotX", 0.0f);
    ActiveViewportClient->Pivot.Y = GetValueFromConfig(config, "OrthoPivotY", 0.0f);
    ActiveViewportClient->Pivot.Z = GetValueFromConfig(config, "OrthoPivotZ", 0.0f);
    ActiveViewportClient->orthoSize = GetValueFromConfig(config, "OrthoZoomSize", 10.0f);
    EditorWidth = GetValueFromConfig(config, "EditorWidth", FEngineLoop::GraphicDevice.screenWidth);
    EditorHeight = GetValueFromConfig(config, "EditorHeight", FEngineLoop::GraphicDevice.screenHeight);

    SetViewportClient(GetValueFromConfig(config, "ActiveViewportIndex", 0));
    bMultiViewportMode = GetValueFromConfig(config, "bMutiView", false);
    for (const auto& ViewportClient : ViewportClients)
    {
        ViewportClient->LoadConfig(config);
    }
    if (HSplitter)
        HSplitter->LoadConfig(config);
    if (VSplitter)
        VSplitter->LoadConfig(config);

}

void SLevelEditor::SaveConfig()
{
    TMap<FString, FString> config;
    if (HSplitter)
        HSplitter->SaveConfig(config);
    if (VSplitter)
        VSplitter->SaveConfig(config);
    for (size_t i = 0; i < 4; i++)
    {
        ViewportClients[i]->SaveConfig(config);
    }
    ActiveViewportClient->SaveConfig(config);
    config["bMutiView"] = std::to_string(bMultiViewportMode);
    config["ActiveViewportIndex"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["ScreenWidth"] = std::to_string(EditorWidth);
    config["ScreenHeight"] = std::to_string(EditorHeight);
    config["OrthoPivotX"] = std::to_string(ActiveViewportClient->Pivot.X);
    config["OrthoPivotY"] = std::to_string(ActiveViewportClient->Pivot.Y);
    config["OrthoPivotZ"] = std::to_string(ActiveViewportClient->Pivot.Z);
    config["OrthoZoomSize"] = std::to_string(ActiveViewportClient->orthoSize);
    WriteIniFile(IniFilePath, config);
}

TMap<FString, FString> SLevelEditor::ReadIniFile(const FString& filePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*filePath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            config[key] = value;
        }
    }
    return config;
}

void SLevelEditor::WriteIniFile(const FString& filePath, const TMap<FString, FString>& config)
{
    std::ofstream file(*filePath);
    for (const auto& pair : config) {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

