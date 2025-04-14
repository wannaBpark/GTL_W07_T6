#include "SWindow.h"


SWindow::SWindow(FRect InRect)
    : Rect(InRect)
{}

void SWindow::Initialize(FRect InitRect)
{
    Rect = InitRect;
}

void SWindow::OnResize(uint32 InWidth, uint32 InHeight)
{
    Rect.Width = InWidth;
    Rect.Height = InHeight;
}

bool SWindow::IsHover(FPoint InPoint) 
{
    return bIsHovered = InPoint.x >= Rect.TopLeftX && InPoint.x < Rect.TopLeftX + Rect.Width &&
        InPoint.y >= Rect.TopLeftY && InPoint.y < Rect.TopLeftY + Rect.Height;
}

bool SWindow::OnPressed(FPoint InPoint)
{
    return false;
}

bool SWindow::OnReleased() 
{
    return false;
}
