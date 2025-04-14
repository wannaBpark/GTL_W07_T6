#pragma once

#include "Define.h"

class SWindow
{
public:
    SWindow() = default;
    SWindow(FRect InRect);
    virtual ~SWindow() = default;

    virtual void Initialize(FRect InitRect);
    virtual void OnResize(uint32 InWidth, uint32 InHeight);

    FRect Rect;
    void SetRect(FRect newRect) { Rect = newRect; }
    
    virtual bool IsHover(FPoint InPoint);
    
    virtual bool OnPressed(FPoint InPoint);
    
    virtual bool OnReleased();
    
    bool IsPressed() const { return bIsPressed; }

protected:
    bool bIsHovered = false;
    bool bIsPressed = false;

};

