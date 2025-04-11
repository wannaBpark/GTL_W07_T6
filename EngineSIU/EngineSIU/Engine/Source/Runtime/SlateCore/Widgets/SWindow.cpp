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
    Rect.width = InWidth;
    Rect.height = InHeight;
}


bool SWindow::IsHover(FPoint InPoint) 
{
    return bIsHovered = InPoint.x >= Rect.leftTopX && InPoint.x < Rect.leftTopX + Rect.width &&
        InPoint.y >= Rect.leftTopY && InPoint.y < Rect.leftTopY + Rect.height;
}

bool SWindow::OnPressed(FPoint InPoint)
{
    return false;
}

bool SWindow::OnReleased() 
{
    return false;
}
