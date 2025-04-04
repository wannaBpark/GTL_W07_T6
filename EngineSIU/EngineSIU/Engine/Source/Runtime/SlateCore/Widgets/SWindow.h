#pragma once

#include "Define.h"

class SWindow
{
public:
    SWindow() = default;
    SWindow(FRect initRect);
    virtual ~SWindow() = default;

    virtual void Initialize(FRect initRect);
    virtual void OnResize(float width, float height);

    FRect Rect;
    void SetRect(FRect newRect) { Rect = newRect; }
    bool IsHover(FPoint coord);
    virtual bool OnPressed(FPoint coord);
    virtual bool OnReleased();
    bool IsPressing() const { return bIsPressed; }

protected:
    bool bIsHoverd = false;
    bool bIsPressed = false;

};

