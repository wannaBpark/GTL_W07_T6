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
        SideLT->Initialize(FRect(0.0f, 0.0f, InRect.LeftTopX, InRect.Height));
    }
    if (SideRB)
    {
        SideRB->Initialize(FRect(InRect.LeftTopX + InRect.Width, 0.0f, InRect.LeftTopX, InRect.Height));
    }
}

void SSplitterH::OnResize(float InWidth, float InHeight)
{
    Rect.Height = InHeight;
    Rect.LeftTopX *= InWidth;
    if (SideLT)
    {
        SideLT->Rect.Height = InHeight;
    }
    if (SideRB)
    {
        SideRB->Rect.LeftTopX *= InWidth;
        SideRB->Rect.Width *= InWidth;
        SideLT->Rect.Height = InHeight;
    }
    UpdateChildRects();
}

void SSplitterH::LoadConfig(const TMap<FString, FString>& config)
{
    // 각 키에 대해 기본값을 지정 (예: 기본 위치 및 크기)
    Rect.LeftTopX = GetValueFromConfig(config, "SplitterH.X", FEngineLoop::GraphicDevice.ScreenWidth *0.5f);
    Rect.LeftTopY = GetValueFromConfig(config, "SplitterH.Y", 0.0f);

    Rect.Width = GetValueFromConfig(config, "SplitterH.Width", 20.0f);
    Rect.Height = GetValueFromConfig(config, "SplitterH.Height", 10.0f); // 수평 스플리터는 높이 고정
    
    Rect.LeftTopX *= FEngineLoop::GraphicDevice.ScreenWidth /GetValueFromConfig(config, "SplitterV.Width", 1000.0f);
}

void SSplitterH::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterH.X"] = std::to_string(Rect.LeftTopX);
    config["SplitterH.Y"] = std::to_string(Rect.LeftTopY);
    config["SplitterH.Width"] = std::to_string(Rect.Width);
    config["SplitterH.Height"] = std::to_string(Rect.Height);
}

void SSplitterH::OnDrag(const FPoint& delta)
{
    // 수평 스플리터의 경우, 좌우로 이동
    Rect.LeftTopX += delta.x;

    UpdateChildRects();
}

void SSplitterH::UpdateChildRects()
{
    if (SideLT)
    {
        SideLT->Rect.Width = Rect.LeftTopX - SideLT->Rect.LeftTopX;
    }
    if (SideRB)
    {
        float prevleftTopX = SideRB->Rect.LeftTopX;
        SideRB->Rect.LeftTopX = Rect.LeftTopX + Rect.Width;
        SideRB->Rect.Width =  SideRB->Rect.Width + prevleftTopX  - SideRB->Rect.LeftTopX;
    }
}

void SSplitterV::Initialize(FRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT)
    {
        SideLT->Initialize(FRect(0.0f, 0.0f, initRect.Width, initRect.LeftTopY));
    }
    if (SideRB)
    {
        SideRB->Initialize(FRect(0.0f, initRect.LeftTopY + initRect.Height, initRect.Width, initRect.LeftTopY));
    }
}

void SSplitterV::OnResize(float width, float height)
{
    Rect.Width = width;
    Rect.LeftTopY *= height;
    if (SideLT)
    {
        SideLT->Rect.Width = width;
    }
    if (SideRB)
    {
        SideRB->Rect.LeftTopY *= height;
        SideRB->Rect.Height *= height;
        SideRB->Rect.Width = width;
    }
    UpdateChildRects();
}

void SSplitterV::LoadConfig(const TMap<FString, FString>& config)
{
    Rect.LeftTopX = GetValueFromConfig(config, "SplitterV.X", 0.0f);
    Rect.LeftTopY = GetValueFromConfig(config, "SplitterV.Y", FEngineLoop::GraphicDevice.ScreenHeight * 0.5f);
    Rect.Width = GetValueFromConfig(config, "SplitterV.Width", 10.f); // 수직 스플리터는 너비 고정
    Rect.Height = GetValueFromConfig(config, "SplitterV.Height", 20.f);

    Rect.LeftTopY *= FEngineLoop::GraphicDevice.ScreenHeight / GetValueFromConfig(config, "SplitterH.Height", 1000.0f);

}

void SSplitterV::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterV.X"] = std::to_string(Rect.LeftTopX);
    config["SplitterV.Y"] = std::to_string(Rect.LeftTopY);
    config["SplitterV.Width"] = std::to_string(Rect.Width);
    config["SplitterV.Height"] = std::to_string(Rect.Height);
}

void SSplitterV::OnDrag(const FPoint& delta)
{
    Rect.LeftTopY += delta.y;
    UpdateChildRects();
}

void SSplitterV::UpdateChildRects()
{
    if (SideLT)
    {
        SideLT->Rect.Height = Rect.LeftTopY - SideLT->Rect.LeftTopY;
    }
    if (SideRB)
    {
        float prevleftTopY = SideRB->Rect.LeftTopY;
        SideRB->Rect.LeftTopY = Rect.LeftTopY + Rect.Height;
        SideRB->Rect.Height = SideRB->Rect.Height + prevleftTopY - SideRB->Rect.LeftTopY;
    }
}
