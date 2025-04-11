#pragma once
#include "Delegates/DelegateCombination.h"
#include "HAL/PlatformType.h"
#include "Math/Vector.h"

namespace EMouseButtons
{
enum Type : uint8;
}

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnKeyCharDelegate, const TCHAR /*Character*/, const bool /*IsRepeat*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnKeyDownDelegate, const int32 /*KeyCode*/, const uint32 /*CharacterCode*/, const bool /*IsRepeat*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnKeyUpDelegate, const int32 /*KeyCode*/, const uint32 /*CharacterCode*/, const bool /*IsRepeat*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMouseDownDelegate, const EMouseButtons::Type /*Button*/, const FVector2D /*CursorPos*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMouseUpDelegate, const EMouseButtons::Type /*Button*/, const FVector2D /*CursorPos*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMouseDoubleClickDelegate, const EMouseButtons::Type /*Button*/, const FVector2D /*CursorPos*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMouseWheelDelegate, const float /*Delta*/, const FVector2D /*CursorPos*/);
DECLARE_MULTICAST_DELEGATE(FOnMouseMoveDelegate);


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
    /** Cursor와 관련된 변수를 업데이트 합니다. */
    void UpdateCursorPosition(const FVector2D& NewPos);

    /** 현재 마우스 포인터의 위치를 가져옵니다. */
    FVector2D GetCursorPos() const;

    /** 한 프레임 전의 마우스 포인터의 위치를 가져옵니다. */
    FVector2D GetLastCursorPos() const;

protected:
    void OnKeyChar(const TCHAR Character, const bool IsRepeat) const;
    void OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) const;
    void OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) const;
    void OnMouseDown(const EMouseButtons::Type Button, const FVector2D CursorPos) const;
    void OnMouseUp(const EMouseButtons::Type Button, const FVector2D CursorPos) const;
    void OnMouseDoubleClick(const EMouseButtons::Type Button, const FVector2D CursorPos) const;
    void OnMouseWheel(const float Delta, const FVector2D CursorPos) const;
    void OnMouseMove() const;
    // 추가적인 함수는 UnrealEngine [SlateApplication.h:1628]을 참조

public:
    FOnKeyCharDelegate OnKeyCharDelegate;
    FOnKeyDownDelegate OnKeyDownDelegate;
    FOnKeyUpDelegate OnKeyUpDelegate;
    FOnMouseDownDelegate OnMouseDownDelegate;
    FOnMouseUpDelegate OnMouseUpDelegate;
    FOnMouseDoubleClickDelegate OnMouseDoubleClickDelegate;
    FOnMouseWheelDelegate OnMouseWheelDelegate;
    FOnMouseMoveDelegate OnMouseMoveDelegate;

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

    // Cursor Position
    FVector2D CurrentPosition;
    FVector2D PreviousPosition;

    bool ModifierKeyState[EModifierKey::Count];
};
