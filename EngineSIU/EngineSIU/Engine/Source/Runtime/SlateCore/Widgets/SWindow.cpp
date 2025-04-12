#include "SWindow.h"


SWindow::SWindow(FRect InRect)
    : Rect(InRect)
{}

void SWindow::Initialize(FRect InitRect)
{
    Rect = InitRect;
}

void SWindow::OnResize(float InWidth, float InHeight)
{
    Rect.Width = InWidth;
    Rect.Height = InHeight;
}


bool SWindow::IsHover(FPoint InPoint) 
{
    return bIsHovered = InPoint.x >= Rect.LeftTopX && InPoint.x < Rect.LeftTopX + Rect.Width &&
        InPoint.y >= Rect.LeftTopY && InPoint.y < Rect.LeftTopY + Rect.Height;
}

bool SWindow::OnPressed(FPoint InPoint)
{
    return false;
}

bool SWindow::OnReleased() 
{
    return false;
}
