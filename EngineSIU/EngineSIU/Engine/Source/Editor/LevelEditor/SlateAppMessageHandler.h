#pragma once
#include "HAL/PlatformType.h"
#include "Math/Vector.h"


// TODO: 나중에 적당한 파일로 옮기기
namespace EMouseButtons
{
enum Type : uint8
{
    Left = 0,
    Middle,
    Right,
    Thumb01,
    Thumb02,

    Invalid,
};
}

class FSlateAppMessageHandler
{
public:
    FSlateAppMessageHandler();

    void ProcessMessage(HWND hWnd, uint32 Msg, WPARAM wParam, LPARAM lParam);

public:
    void OnKeyChar(const TCHAR Character, const bool IsRepeat) const;
    void OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) const;
    void OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) const;
    void OnMouseDown(const EMouseButtons::Type Button, const FVector2D CursorPos) const;
    void OnMouseUp(const EMouseButtons::Type Button, const FVector2D CursorPos) const;
    void OnMouseDoubleClick(const EMouseButtons::Type Button, const FVector2D CursorPos) const;
    void OnMouseWheel(const float Delta, const FVector2D CursorPos) const;
    void OnMouseMove() const;

    FVector2D GetCursorPos() const;
    FVector2D GetLastCursorPos() const;

private:
    struct EModifierKey
    {
        enum Type : uint8
        {
            LeftShift,    // VK_LSHIFT
            RightShift,   // VK_RSHIFT
            LeftControl,  // VK_LCONTROL
            RightControl, // VK_RCONTROL
            LeftAlt,      // VK_LMENU
            RightAlt,     // VK_RMENU
            CapsLock,     // VK_CAPITAL
            Count,
        };
    };

    FVector2D CurrentPosition;
    FVector2D PreviousPosition;

    bool ModifierKeyState[EModifierKey::Count];
};
