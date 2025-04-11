#pragma once

#include "Define.h"

class SWindow
{
public:
    SWindow() = default;
    SWindow(FRect InRect);
    virtual ~SWindow() = default;

    virtual void Initialize(FRect InitRect);
    virtual void OnResize(float InWidth, float InHeight);

    FRect Rect;
    void SetRect(FRect newRect) { Rect = newRect; }
    
    bool IsHover(FPoint InPoint);
    
    virtual bool OnPressed(FPoint InPoint);
    
    virtual bool OnReleased();
    
    bool IsPressed() const { return bIsPressed; }

protected:
    bool bIsHovered = false;
    bool bIsPressed = false;

};

