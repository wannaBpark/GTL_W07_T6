#include "SLevelEditor.h"
#include <fstream>
#include <ostream>
#include <sstream>
#include "EngineLoop.h"
#include "UnrealClient.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "SlateCore/Widgets/SWindow.h"
#include "UnrealEd/EditorViewportClient.h"

SLevelEditor::SLevelEditor()
    : bInitialized(false)
    , HSplitter(nullptr)
    , VSplitter(nullptr)
    , bMultiViewportMode(false)
{
}

void SLevelEditor::Initialize(uint32 InEditorWidth, uint32 InEditorHeight)
{
    ResizeEditor(InEditorWidth, InEditorHeight);
    
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
    
    bInitialized = true;
}

void SLevelEditor::Tick(float DeltaTime)
{
    if (bMultiViewportMode)
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GEngineLoop.hWnd, &pt);
        if (VSplitter->IsHover(FPoint(pt.x, pt.y)) || HSplitter->IsHover(FPoint(pt.x, pt.y)))
        {
            SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
        }
        else
        {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        Input();
    }
    
    ActiveViewportClient->Input();
    for (const std::shared_ptr<FEditorViewportClient>& Viewport : ViewportClients)
    {
        Viewport->Tick(DeltaTime);
    }
}

void SLevelEditor::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (bLButtonDown == false)
        {
            bLButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&PrevCursorLocation);
            ScreenToClient(GEngineLoop.hWnd, &pt);

            SelectViewport(pt);

            VSplitter->OnPressed(FPoint(pt.x, pt.y));
            HSplitter->OnPressed(FPoint(pt.x, pt.y));
        }
        else
        {
            POINT currentMousePos;
            GetCursorPos(&currentMousePos);

            // 마우스 이동 차이 계산
            int32 DeltaX = currentMousePos.x - PrevCursorLocation.x;
            int32 DeltaY = currentMousePos.y - PrevCursorLocation.y;

            if (VSplitter->IsPressed())
            {
                VSplitter->OnDrag(FPoint(DeltaX, DeltaY));
            }
            if (HSplitter->IsPressed())
            {
                HSplitter->OnDrag(FPoint(DeltaX, DeltaY));
            }
            ResizeViewports();
            PrevCursorLocation = currentMousePos;
        }
    }
    else
    {
        bLButtonDown = false;
        VSplitter->OnReleased();
        HSplitter->OnReleased();
    }
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (!bRButtonDown)
        {
            bRButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&PrevCursorLocation);
            ScreenToClient(GEngineLoop.hWnd, &pt);

            SelectViewport(pt);
        }
    }
    else
    {
        bRButtonDown = false;
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
    
    const uint32 PrevEditorWidth = EditorWidth;
    const uint32 PrevEditorHeight = EditorHeight;
    EditorWidth = InEditorWidth;
    EditorHeight = InEditorHeight;

    if (bInitialized)
    {
        //HSplitter 에는 바뀐 width 비율이 들어감 
        HSplitter->OnResize(EditorWidth / PrevEditorWidth, EditorHeight);
        //HSplitter 에는 바뀐 Height 비율이 들어감 
        VSplitter->OnResize(EditorWidth, EditorHeight / PrevEditorHeight);
        ResizeViewports();
    }
}

void SLevelEditor::SelectViewport(POINT point)
{
    for (int i = 0; i < 4; i++)
    {
        if (ViewportClients[i]->IsSelected(point))
        {
            SetActiveViewportClient(i);
            break;
        }
    }
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

void SLevelEditor::EnableMultiViewport()
{
    bMultiViewportMode = true;
    ResizeViewports();
}

void SLevelEditor::DisableMultiViewport()
{
    bMultiViewportMode = false;
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
    config["ScreenWidth"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["ScreenHeight"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["OrthoPivotX"] = std::to_string(ActiveViewportClient->Pivot.X);
    config["OrthoPivotY"] = std::to_string(ActiveViewportClient->Pivot.Y);
    config["OrthoPivotZ"] = std::to_string(ActiveViewportClient->Pivot.Z);
    config["OrthoZoomSize"] = std::to_string(ActiveViewportClient->OrthoSize);
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

