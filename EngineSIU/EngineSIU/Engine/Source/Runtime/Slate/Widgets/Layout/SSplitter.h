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
    
    virtual void Initialize(FRect initRect) override;

    virtual void OnDragStart(const FPoint& mousePos) { /* 초기화 */ }
    virtual void OnDrag(const FPoint& delta) = 0; // 가로/세로에 따라 구현 다름.
    virtual void OnResize(float Width, float Height) override;
    virtual bool OnPressed(FPoint InPoint) override;
    virtual bool OnReleased() override;

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
    virtual void OnResize(float InWidth, float InHeight) override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;

    virtual void OnDrag(const FPoint& delta) override;

    virtual void UpdateChildRects() override;
};

class SSplitterV : public SSplitter
{
public:
    virtual void Initialize(FRect initRect) override;
    virtual void OnResize(float width, float height) override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;

    virtual void OnDrag(const FPoint& delta) override;

    virtual void UpdateChildRects() override;
};
