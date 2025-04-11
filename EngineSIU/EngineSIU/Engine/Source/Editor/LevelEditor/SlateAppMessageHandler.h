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

/**
 * ModifierKeys를 구성하는 키의 상태를 나타내는 구조체입니다.
 */
struct FModifierKeysState
{
public:
    FModifierKeysState(
        const bool bInIsLeftShiftDown,
        const bool bInIsRightShiftDown,
        const bool bInIsLeftControlDown,
        const bool bInIsRightControlDown,
        const bool bInIsLeftAltDown,
        const bool bInIsRightAltDown,
        const bool bInIsLeftWinDown,
        const bool bInIsRightWinDown,
        const bool bInAreCapsLocked
    )
        : bIsLeftShiftDown(static_cast<uint16>(bInIsLeftShiftDown))
        , bIsRightShiftDown(static_cast<uint16>(bInIsRightShiftDown))
        , bIsLeftControlDown(static_cast<uint16>(bInIsLeftControlDown))
        , bIsRightControlDown(static_cast<uint16>(bInIsRightControlDown))
        , bIsLeftAltDown(static_cast<uint16>(bInIsLeftAltDown))
        , bIsRightAltDown(static_cast<uint16>(bInIsRightAltDown))
        , bIsLeftWinDown(static_cast<uint16>(bInIsLeftWinDown))
        , bIsRightWinDown(static_cast<uint16>(bInIsRightWinDown))
        , bAreCapsLocked(static_cast<uint16>(bInAreCapsLocked))
    {
    }

    FModifierKeysState()
        : bIsLeftShiftDown(false)
        , bIsRightShiftDown(false)
        , bIsLeftControlDown(false)
        , bIsRightControlDown(false)
        , bIsLeftAltDown(false)
        , bIsRightAltDown(false)
        , bIsLeftWinDown(false)
        , bIsRightWinDown(false)
        , bAreCapsLocked(false)
    {
    }

    /**
     * Shift 키 중 하나라도 눌려 있었는지 여부를 반환합니다.
     *
     * @return  Shift 키가 눌려 있으면 true
     */
    bool IsShiftDown() const
    {
        return bIsLeftShiftDown || bIsRightShiftDown;
    }

    /**
     * 왼쪽 Shift 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  왼쪽 Shift 키가 눌려 있으면 true
     */
    bool IsLeftShiftDown() const
    {
        return bIsLeftShiftDown;
    }

    /**
     * 오른쪽 Shift 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  오른쪽 Shift 키가 눌려 있으면 true
     */
    bool IsRightShiftDown() const
    {
        return bIsRightShiftDown;
    }

    /**
     * Control 키 중 하나라도 눌려 있었는지 여부를 반환합니다.
     *
     * @return  Control 키가 눌려 있으면 true
     */
    bool IsControlDown() const
    {
        return bIsLeftControlDown || bIsRightControlDown;
    }

    /**
     * 왼쪽 Control 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  왼쪽 Control 키가 눌려 있으면 true
     */
    bool IsLeftControlDown() const
    {
        return bIsLeftControlDown;
    }

    /**
     * 오른쪽 Control 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  오른쪽 Control 키가 눌려 있으면 true
     */
    bool IsRightControlDown() const
    {
        return bIsRightControlDown;
    }

    /**
     * Alt 키 중 하나라도 눌려 있었는지 여부를 반환합니다.
     *
     * @return  Alt 키가 눌려 있으면 true
     */
    bool IsAltDown() const
    {
        return bIsLeftAltDown || bIsRightAltDown;
    }

    /**
     * 왼쪽 Alt 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  왼쪽 Alt 키가 눌려 있으면 true
     */
    bool IsLeftAltDown() const
    {
        return bIsLeftAltDown;
    }

    /**
     * 오른쪽 Alt 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  오른쪽 Alt 키가 눌려 있으면 true
     */
    bool IsRightAltDown() const
    {
        return bIsRightAltDown;
    }

    /**
     * Windows 키 중 하나라도 눌려 있었는지 여부를 반환합니다.
     *
     * @return  Windows 키가 눌려 있으면 true
     */
    bool IsWindowsKeyDown() const
    {
        return bIsLeftWinDown || bIsRightWinDown;
    }

    /**
     * 왼쪽 Windows 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  왼쪽 Windows 키가 눌려 있으면 true
     */
    bool IsLeftWinDown() const
    {
        return bIsLeftWinDown;
    }

    /**
     * 오른쪽 Windows 키가 눌려 있었는지 여부를 반환합니다.
     *
     * @return  오른쪽 Windows 키가 눌려 있으면 true
     */
    bool IsRightWinDown() const
    {
        return bIsRightWinDown;
    }

    /**
     * Caps Lock 키가 활성화 상태인지 여부를 반환합니다.
     *
     * @return  Caps Lock이 활성화되어 있으면 true
     */
    bool AreCapsLocked() const
    {
        return bAreCapsLocked;
    }


private:
    uint16 bIsLeftShiftDown : 1;
    uint16 bIsRightShiftDown : 1;
    uint16 bIsLeftControlDown : 1;
    uint16 bIsRightControlDown : 1;
    uint16 bIsLeftAltDown : 1;
    uint16 bIsRightAltDown : 1;
    uint16 bIsLeftWinDown : 1;
    uint16 bIsRightWinDown : 1;
    uint16 bAreCapsLocked : 1;
};

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

    /** ModifierKeys의 상태를 가져옵니다. */
    FModifierKeysState GetModifierKeys() const;

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
            LeftWin,      // VK_LWIN
            RightWin,     // VK_RWIN
            CapsLock,     // VK_CAPITAL
            Count,
        };
    };

    // Cursor Position
    FVector2D CurrentPosition;
    FVector2D PreviousPosition;

    bool ModifierKeyState[EModifierKey::Count];
};
