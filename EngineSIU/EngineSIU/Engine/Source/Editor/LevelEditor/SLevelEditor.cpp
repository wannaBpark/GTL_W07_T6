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
    , bMultiViewportMode(false)
{
}

void SLevelEditor::Initialize(uint32 InEditorWidth, uint32 InEditorHeight)
{
    ResizeEditor(InEditorWidth, InEditorHeight);
    
    VSplitter = new SSplitterV();
    VSplitter->Initialize(FRect(0.0f, 0.f, InEditorWidth, InEditorHeight));
    
    HSplitter = new SSplitterH();
    HSplitter->Initialize(FRect(0.f, 0.0f, InEditorWidth, InEditorHeight));
    
    FRect Top = VSplitter->SideLT->GetRect();
    FRect Bottom = VSplitter->SideRB->GetRect();
    FRect Left = HSplitter->SideLT->GetRect();
    FRect Right = HSplitter->SideRB->GetRect();

    for (size_t i = 0; i < 4; i++)
    {
        EViewScreenLocation Location = static_cast<EViewScreenLocation>(i);
        FRect Rect;
        switch (Location)
        {
        case EViewScreenLocation::EVL_TopLeft:
            Rect.TopLeftX = Left.TopLeftX;
            Rect.TopLeftY = Top.TopLeftY;
            Rect.Width = Left.Width;
            Rect.Height = Top.Height;
            break;
        case EViewScreenLocation::EVL_TopRight:
            Rect.TopLeftX = Right.TopLeftX;
            Rect.TopLeftY = Top.TopLeftY;
            Rect.Width = Right.Width;
            Rect.Height = Top.Height;
            break;
        case EViewScreenLocation::EVL_BottomLeft:
            Rect.TopLeftX = Left.TopLeftX;
            Rect.TopLeftY = Bottom.TopLeftY;
            Rect.Width = Left.Width;
            Rect.Height = Bottom.Height;
            break;
        case EViewScreenLocation::EVL_BottomRight:
            Rect.TopLeftX = Right.TopLeftX;
            Rect.TopLeftY = Bottom.TopLeftY;
            Rect.Width = Right.Width;
            Rect.Height = Bottom.Height;
            break;
        default:
            return;
        }
        ViewportClients[i] = std::make_shared<FEditorViewportClient>();
        ViewportClients[i]->Initialize(Location, Rect);
    }
    
    ActiveViewportClient = ViewportClients[0];
    
    LoadConfig();

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
        {
        case EKeys::RightMouseButton:
        {
            if (!bIsPressedMouseRightButton && !InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
            {
                FWindowsCursor::SetShowMouseCursor(false);
                MousePinPosition = InMouseEvent.GetScreenSpacePosition();
                bIsPressedMouseRightButton = true;
            }
            break;
        }
        default:
            break;
        }

        // 마우스 이벤트가 일어난 위치의 뷰포트를 선택
        if (bMultiViewportMode)
        {
            POINT Point;
            GetCursorPos(&Point);
            ScreenToClient(GEngineLoop.AppWnd, &Point);
            FVector2D ClientPos = FVector2D{static_cast<float>(Point.x), static_cast<float>(Point.y)};
            SelectViewport(ClientPos);
            VSplitter->OnPressed({ClientPos.X, ClientPos.Y});
            HSplitter->OnPressed({ClientPos.X, ClientPos.Y});
        }
    });

    Handler->OnMouseMoveDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        // Splitter 움직임 로직
        if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
        {
            const auto& [DeltaX, DeltaY] = InMouseEvent.GetCursorDelta();
            
            bool bSplitterDragging = false;
            if (VSplitter->IsSplitterPressed())
            {
                VSplitter->OnDrag(FPoint(DeltaX, DeltaY));
                bSplitterDragging = true;
            }
            if (HSplitter->IsSplitterPressed())
            {
                HSplitter->OnDrag(FPoint(DeltaX, DeltaY));
                bSplitterDragging = true;
            }

            if (bSplitterDragging)
            {
                ResizeViewports();
            }
        }

        // 멀티 뷰포트일 때, 커서 변경 로직
        if (
            bMultiViewportMode
            && !InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)
            && !InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)
        ) {
            // TODO: 나중에 커서가 Viewport 위에 있을때만 ECursorType::Crosshair로 바꾸게끔 하기
            // ECursorType CursorType = ECursorType::Crosshair;
            ECursorType CursorType = ECursorType::Arrow;
            POINT Point;

            GetCursorPos(&Point);
            ScreenToClient(GEngineLoop.AppWnd, &Point);
            FVector2D ClientPos = FVector2D{static_cast<float>(Point.x), static_cast<float>(Point.y)};
            const bool bIsVerticalHovered = VSplitter->IsSplitterHovered({ClientPos.X, ClientPos.Y});
            const bool bIsHorizontalHovered = HSplitter->IsSplitterHovered({ClientPos.X, ClientPos.Y});

            if (bIsHorizontalHovered && bIsVerticalHovered)
            {
                CursorType = ECursorType::ResizeAll;
            }
            else if (bIsHorizontalHovered)
            {
                CursorType = ECursorType::ResizeLeftRight;
            }
            else if (bIsVerticalHovered)
            {
                CursorType = ECursorType::ResizeUpDown;
            }
            FWindowsCursor::SetMouseCursor(CursorType);
        }
    });

    Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
        {
        case EKeys::RightMouseButton:
        {
            if (bIsPressedMouseRightButton)
            {
                FWindowsCursor::SetShowMouseCursor(true);
                FWindowsCursor::SetPosition(
                    static_cast<int32>(MousePinPosition.X),
                    static_cast<int32>(MousePinPosition.Y)
                );
                bIsPressedMouseRightButton = false;
            }
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

    Handler->OnRawMouseInputDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        // Mouse Move 이벤트 일때만 실행
        if (
            InMouseEvent.GetInputEvent() == IE_Axis
            && InMouseEvent.GetEffectingButton() == EKeys::Invalid
        )
        {
            // 에디터 카메라 이동 로직
            if (bIsPressedMouseRightButton)
            {
                ActiveViewportClient->MouseMove(InMouseEvent);
            }
        }

        // 마우스 휠 이벤트
        else if (InMouseEvent.GetEffectingButton() == EKeys::MouseWheelAxis)
        {
            // 카메라 속도 조절
            if (bIsPressedMouseRightButton && ActiveViewportClient->IsPerspective())
            {
                const float CurrentSpeed = ActiveViewportClient->GetCameraSpeedScalar();
                const float Adjustment = FMath::Sign(InMouseEvent.GetWheelDelta()) * FMath::Loge(CurrentSpeed + 1.0f) * 0.5f;

                ActiveViewportClient->SetCameraSpeed(CurrentSpeed + Adjustment);
            }
        }
    });

    Handler->OnMouseWheelDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        // 뷰포트에서 앞뒤 방향으로 화면 이동
        if (ActiveViewportClient->IsPerspective())
        {
            if (!bIsPressedMouseRightButton)
            {
                const FVector CameraLoc = ActiveViewportClient->PerspectiveCamera.GetLocation();
                const FVector CameraForward = ActiveViewportClient->PerspectiveCamera.GetForwardVector();
                ActiveViewportClient->PerspectiveCamera.SetLocation(
                    CameraLoc + CameraForward * InMouseEvent.GetWheelDelta() * 50.0f
                );
            }
        }
        else
        {
            FEditorViewportClient::SetOthoSize(-InMouseEvent.GetWheelDelta());
        }
    });

    Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        ActiveViewportClient->InputKey(InKeyEvent);
    });

    Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        ActiveViewportClient->InputKey(InKeyEvent);
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

void SLevelEditor::ResizeEditor(uint32 InEditorWidth, uint32 InEditorHeight)
{
    if (InEditorWidth == EditorWidth && InEditorHeight == EditorHeight)
    {
        return;
    }
    
    EditorWidth = InEditorWidth;
    EditorHeight = InEditorHeight;

    if (HSplitter && VSplitter)
    {
        HSplitter->OnResize(EditorWidth, EditorHeight);
        VSplitter->OnResize(EditorWidth, EditorHeight);
        ResizeViewports();
    }
}

void SLevelEditor::SelectViewport(const FVector2D& Point)
{
    for (int i = 0; i < 4; i++)
    {
        if (ViewportClients[i]->IsSelected(Point))
        {
            SetActiveViewportClient(i);
            return;
        }
    }
}

void SLevelEditor::ResizeViewports()
{
    if (bMultiViewportMode)
    {
        if (GetViewports()[0])
        {
            for (int i = 0; i < 4; ++i)
            {
                GetViewports()[i]->ResizeViewport(
                    VSplitter->SideLT->GetRect(),
                    VSplitter->SideRB->GetRect(),
                    HSplitter->SideLT->GetRect(),
                    HSplitter->SideRB->GetRect()
                );
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
    auto Config = ReadIniFile(IniFilePath);
    FEditorViewportClient::Pivot.X = GetValueFromConfig(Config, "OrthoPivotX", 0.0f);
    FEditorViewportClient::Pivot.Y = GetValueFromConfig(Config, "OrthoPivotY", 0.0f);
    FEditorViewportClient::Pivot.Z = GetValueFromConfig(Config, "OrthoPivotZ", 0.0f);
    FEditorViewportClient::OrthoSize = GetValueFromConfig(Config, "OrthoZoomSize", 10.0f);

    SetActiveViewportClient(GetValueFromConfig(Config, "ActiveViewportIndex", 0));
    bMultiViewportMode = GetValueFromConfig(Config, "bMultiView", false);
    if (bMultiViewportMode)
    {
        SetEnableMultiViewport(true);
    }
    else
    {
        SetEnableMultiViewport(false);
    }
    
    for (size_t i = 0; i < 4; i++)
    {
        ViewportClients[i]->LoadConfig(Config);
    }
    
    if (HSplitter)
    {
        HSplitter->LoadConfig(Config);
    }
    if (VSplitter)
    {
        VSplitter->LoadConfig(Config);
    }

    ResizeViewports();
}

void SLevelEditor::SaveConfig()
{
    TMap<FString, FString> config;
    if (HSplitter)
    {
        HSplitter->SaveConfig(config);
    }
    if (VSplitter)
    {
        VSplitter->SaveConfig(config);
    }
    for (size_t i = 0; i < 4; i++)
    {
        ViewportClients[i]->SaveConfig(config);
    }
    ActiveViewportClient->SaveConfig(config);
    config["bMultiView"] = std::to_string(bMultiViewportMode);
    config["ActiveViewportIndex"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["ScreenWidth"] = std::to_string(EditorWidth);
    config["ScreenHeight"] = std::to_string(EditorHeight);
    config["OrthoPivotX"] = std::to_string(ActiveViewportClient->Pivot.X);
    config["OrthoPivotY"] = std::to_string(ActiveViewportClient->Pivot.Y);
    config["OrthoPivotZ"] = std::to_string(ActiveViewportClient->Pivot.Z);
    config["OrthoZoomSize"] = std::to_string(ActiveViewportClient->OrthoSize);
    WriteIniFile(IniFilePath, config);
}

TMap<FString, FString> SLevelEditor::ReadIniFile(const FString& FilePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*FilePath);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == ';')
        {
            continue;
        }
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            config[key] = value;
        }
    }
    return config;
}

void SLevelEditor::WriteIniFile(const FString& FilePath, const TMap<FString, FString>& Config)
{
    std::ofstream file(*FilePath);
    for (const auto& pair : Config)
    {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

