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

void SSplitter::OnResize(uint32 InWidth, uint32 InHeight)
{
    Rect.Width = static_cast<float>(InWidth);
    Rect.Height = static_cast<float>(InHeight);
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

    UpdateChildRects();
}

void SSplitterH::ClampSplitRatio()
{
    SplitRatio = FMath::Max(SplitRatio, static_cast<float>(SplitterLimitLT) / Rect.Width);
    SplitRatio = FMath::Min(SplitRatio, (Rect.Width - static_cast<float>(SplitterLimitLT)) / Rect.Width);
}

float SSplitterH::GetSplitterLTCenter()
{
    ClampSplitRatio();
    return Rect.Width * SplitRatio;
}

void SSplitterH::LoadConfig(const TMap<FString, FString>& config)
{
    // 각 키에 대해 기본값을 지정 (예: 기본 위치 및 크기)
    Rect.TopLeftX = GetValueFromConfig(config, "SplitterH.X", FEngineLoop::GraphicDevice.ScreenWidth * 0.5f);
    Rect.TopLeftY = GetValueFromConfig(config, "SplitterH.Y", 0.0f);

    Rect.Width = GetValueFromConfig(config, "SplitterH.Width", 20.0f);
    Rect.Height = GetValueFromConfig(config, "SplitterH.Height", 10.0f); // 수평 스플리터는 높이 고정
    
    // Rect.TopLeftX *= FEngineLoop::GraphicDevice.ScreenWidth / GetValueFromConfig(config, "SplitterV.Width", 1000.0f);
}

void SSplitterH::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterH.X"] = std::to_string(Rect.TopLeftX);
    config["SplitterH.Y"] = std::to_string(Rect.TopLeftY);
    config["SplitterH.Width"] = std::to_string(Rect.Width);
    config["SplitterH.Height"] = std::to_string(Rect.Height);
}

void SSplitterH::OnResize(uint32 InWidth, uint32 InHeight)
{
    __super::OnResize(InWidth, InHeight);
    
    UpdateChildRects();
}

void SSplitterH::OnDrag(const FPoint& Delta)
{
    // 수평 스플리터의 경우, 좌우로 이동
    float CenterX = GetSplitterLTCenter();
    CenterX += Delta.x;

    SplitRatio = CenterX / Rect.Width;
    
    UpdateChildRects();
}

void SSplitterH::UpdateChildRects()
{
    const float SplitterCenterX = GetSplitterLTCenter();
    
    if (SideLT)
    {
        SideLT->Initialize(FRect(
            0.0f, 0.0f,
            SplitterCenterX - SplitterHalfThickness, Rect.Height
        ));
    }
    if (SideRB)
    {
        const float Offset = SplitterCenterX + SplitterHalfThickness;
        
        SideRB->Initialize(FRect(
            Offset, 0.0f,
            Rect.Width - Offset, Rect.Height
        ));
    }
}

void SSplitterV::Initialize(FRect InRect)
{
    __super::Initialize(InRect);

    UpdateChildRects();
}

void SSplitterV::ClampSplitRatio()
{
    SplitRatio = FMath::Max(SplitRatio, static_cast<float>(SplitterLimitLT) / Rect.Height);
    SplitRatio = FMath::Min(SplitRatio, (Rect.Height - static_cast<float>(SplitterLimitLT)) / Rect.Height);
}

float SSplitterV::GetSplitterLTCenter()
{
    ClampSplitRatio();
    return Rect.Height * SplitRatio;
}

void SSplitterV::LoadConfig(const TMap<FString, FString>& config)
{
    Rect.TopLeftX = GetValueFromConfig(config, "SplitterV.X", 0.0f);
    Rect.TopLeftY = GetValueFromConfig(config, "SplitterV.Y", FEngineLoop::GraphicDevice.ScreenHeight * 0.5f);
    Rect.Width = GetValueFromConfig(config, "SplitterV.Width", 10.f); // 수직 스플리터는 너비 고정
    Rect.Height = GetValueFromConfig(config, "SplitterV.Height", 20.f);

    // Rect.TopLeftY *= FEngineLoop::GraphicDevice.ScreenHeight / GetValueFromConfig(config, "SplitterH.Height", 1000.0f);
}

void SSplitterV::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterV.X"] = std::to_string(Rect.TopLeftX);
    config["SplitterV.Y"] = std::to_string(Rect.TopLeftY);
    config["SplitterV.Width"] = std::to_string(Rect.Width);
    config["SplitterV.Height"] = std::to_string(Rect.Height);
}

void SSplitterV::OnResize(uint32 InWidth, uint32 InHeight)
{
    __super::OnResize(InWidth, InHeight);
    
    UpdateChildRects();
}

void SSplitterV::OnDrag(const FPoint& Delta)
{
    float CenterY = GetSplitterLTCenter();
    CenterY += Delta.y;

    SplitRatio = CenterY / Rect.Height;
    
    UpdateChildRects();
}

void SSplitterV::UpdateChildRects()
{
    const float SplitterCenterY = GetSplitterLTCenter();
    
    if (SideLT)
    {
        SideLT->Initialize(FRect(
            0.0f, 0.0f,
            Rect.Width, SplitterCenterY - SplitterHalfThickness
        ));
    }
    if (SideRB)
    {
        const float Offset = SplitterCenterY + SplitterHalfThickness;
        
        SideRB->Initialize(FRect(
            0.0f, Offset,
            Rect.Width, Rect.Height - Offset
        ));
    }
}
