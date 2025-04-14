#pragma once
#include "SlateCore/Widgets/SWindow.h"
#include "Container/Map.h"
#include "fstream"
#include "sstream"

class SSplitter : public SWindow
{
public:
    SWindow* SideLT; // Left or Top
    SWindow* SideRB; // Right or Bottom

    float SplitRatio = 0.5f; // 값 범위: [0, 1]

    uint32 SplitterLimitLT = 100; // Pixel Value

    float SplitterHalfThickness = 5.f; // Pixel Value

    bool bIsSplitterPressed = false;

    virtual void Initialize(FRect initRect) override;

    virtual void ClampSplitRatio() {}
    
    virtual float GetSplitterLTCenter() = 0;

    virtual void OnDragStart(const FPoint& mousePos) { /* 초기화 */ }
    virtual void OnDrag(const FPoint& delta) = 0; // 가로/세로에 따라 구현 다름.
    virtual void OnResize(uint32 InWidth, uint32 InHeight) override;
    virtual bool OnPressed(const FPoint& InPoint) override;
    virtual bool OnReleased() override;

    virtual bool IsSplitterHovered(const FPoint& InPoint) const;

    virtual bool IsSplitterPressed() const { return bIsSplitterPressed; }

    virtual void OnDragEnd() {}

    virtual void LoadConfig(const TMap<FString, FString>& Config);
    virtual void SaveConfig(TMap<FString, FString>& Config) const;

    // 스플리터가 포함된 영역에 따라 자식 창의 Rect를 재계산하는 함수
    virtual void UpdateChildRects() = 0;

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};

class SSplitterH : public SSplitter
{
public:
    virtual void Initialize(FRect InRect) override;

    virtual void ClampSplitRatio() override;
    
    virtual float GetSplitterLTCenter() override;
    
    virtual void OnResize(uint32 InWidth, uint32 InHeight) override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;

    virtual void OnDrag(const FPoint& Delta) override;

    virtual void UpdateChildRects() override;
};

class SSplitterV : public SSplitter
{
public:
    virtual void Initialize(FRect InRect) override;

    virtual void ClampSplitRatio() override;

    virtual float GetSplitterLTCenter() override;
    
    virtual void OnResize(uint32 InWidth, uint32 InHeight) override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;

    virtual void OnDrag(const FPoint& Delta) override;

    virtual void UpdateChildRects() override;
};
