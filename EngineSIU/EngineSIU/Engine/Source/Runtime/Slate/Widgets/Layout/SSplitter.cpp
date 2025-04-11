#include "SSplitter.h"
#include "EngineLoop.h"

extern FEngineLoop GEngineLoop;

void SSplitter::Initialize(FRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT == nullptr)
    {
        SideLT = new SWindow();
    }
    if (SideRB == nullptr)
    {
        SideRB = new SWindow();
    }
}

void SSplitter::OnResize(float Width, float Height)
{
}

bool SSplitter::OnPressed(FPoint InPoint)
{
    if (!IsHover(InPoint))
        return false;
    
    return bIsPressed = true;
}

bool SSplitter::OnReleased()
{
    bIsPressed = false;
    return false;
}

void SSplitter::LoadConfig(const TMap<FString, FString>& Config)
{
}

void SSplitter::SaveConfig(TMap<FString, FString>& Config) const
{
}

void SSplitterH::Initialize(FRect InRect)
{
    __super::Initialize(InRect);
    if (SideLT)
    {
        SideLT->Initialize(FRect(0.0f, 0.0f, InRect.leftTopX, InRect.height));
    }
    if (SideRB)
    {
        SideRB->Initialize(FRect(InRect.leftTopX + InRect.width, 0.0f, InRect.leftTopX, InRect.height));
    }
}

void SSplitterH::OnResize(float InWidth, float InHeight)
{
    Rect.height = InHeight;
    Rect.leftTopX *= InWidth;
    if (SideLT)
    {
        SideLT->Rect.height = InHeight;
    }
    if (SideRB)
    {
        SideRB->Rect.leftTopX *= InWidth;
        SideRB->Rect.width *= InWidth;
        SideLT->Rect.height = InHeight;
    }
    UpdateChildRects();
}

void SSplitterH::LoadConfig(const TMap<FString, FString>& config)
{
    // 각 키에 대해 기본값을 지정 (예: 기본 위치 및 크기)
    Rect.leftTopX = GetValueFromConfig(config, "SplitterH.X", FEngineLoop::GraphicDevice.ScreenWidth *0.5f);
    Rect.leftTopY = GetValueFromConfig(config, "SplitterH.Y", 0.0f);

    Rect.width = GetValueFromConfig(config, "SplitterH.Width", 20.0f);
    Rect.height = GetValueFromConfig(config, "SplitterH.Height", 10.0f); // 수평 스플리터는 높이 고정
    
    Rect.leftTopX *= FEngineLoop::GraphicDevice.ScreenWidth /GetValueFromConfig(config, "SplitterV.Width", 1000.0f);
}

void SSplitterH::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterH.X"] = std::to_string(Rect.leftTopX);
    config["SplitterH.Y"] = std::to_string(Rect.leftTopY);
    config["SplitterH.Width"] = std::to_string(Rect.width);
    config["SplitterH.Height"] = std::to_string(Rect.height);
}

void SSplitterV::Initialize(FRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT)
    {
        SideLT->Initialize(FRect(0.0f, 0.0f, initRect.width, initRect.leftTopY));
    }
    if (SideRB)
    {
        SideRB->Initialize(FRect(0.0f, initRect.leftTopY + initRect.height, initRect.width, initRect.leftTopY));
    }
}

void SSplitterV::OnResize(float width, float height)
{
    Rect.width = width;
    Rect.leftTopY *= height;
    if (SideLT)
    {
        SideLT->Rect.width = width;
    }
    if (SideRB)
    {
        SideRB->Rect.leftTopY *= height;
        SideRB->Rect.height *= height;
        SideRB->Rect.width = width;
    }
    UpdateChildRects();
}

void SSplitterV::LoadConfig(const TMap<FString, FString>& config)
{
    Rect.leftTopX = GetValueFromConfig(config, "SplitterV.X", 0.0f);
    Rect.leftTopY = GetValueFromConfig(config, "SplitterV.Y", FEngineLoop::GraphicDevice.ScreenHeight * 0.5f);
    Rect.width = GetValueFromConfig(config, "SplitterV.Width", 10.f); // 수직 스플리터는 너비 고정
    Rect.height = GetValueFromConfig(config, "SplitterV.Height", 20.f);

    Rect.leftTopY *= FEngineLoop::GraphicDevice.ScreenHeight / GetValueFromConfig(config, "SplitterH.Height", 1000.0f);

}

void SSplitterV::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterV.X"] = std::to_string(Rect.leftTopX);
    config["SplitterV.Y"] = std::to_string(Rect.leftTopY);
    config["SplitterV.Width"] = std::to_string(Rect.width);
    config["SplitterV.Height"] = std::to_string(Rect.height);
}
