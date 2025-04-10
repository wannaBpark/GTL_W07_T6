#include "SlateAppMessageHandler.h"

#define _TCHAR_DEFINED
#include <windowsx.h>

#include "Define.h"
#include "Math/Vector.h"


FSlateAppMessageHandler::FSlateAppMessageHandler()
{
    for (bool& KeyState : ModifierKeyState)
    {
        KeyState = false;
    }
}

void FSlateAppMessageHandler::ProcessMessage(HWND hWnd, uint32 Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_CHAR:
    {
        // WPARAM으로부터 문자 가져오기
        const TCHAR Character = static_cast<TCHAR>(wParam);

        // lParam의 30번째 비트(0x40000000)는 키가 계속 눌려져 있는 상태(키 반복)인지 확인
        const bool bIsRepeat = ( lParam & 0x40000000 ) != 0;
        OnKeyChar(Character, bIsRepeat);
        return;
    }

    // 키보드 키가 눌렸을 때 발생하는 메시지
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        const int32 Win32Key = static_cast<int32>(wParam);
        int32 ActualKey = Win32Key;

        bool bIsRepeat = (lParam & 0x40000000) != 0;

        switch (Win32Key)
        {
        case VK_MENU:
            // Differentiate between left and right alt
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LMENU;
                bIsRepeat = ModifierKeyState[EModifierKey::LeftAlt];
                ModifierKeyState[EModifierKey::LeftAlt] = true;
            }
            else
            {
                ActualKey = VK_RMENU;
                bIsRepeat = ModifierKeyState[EModifierKey::RightAlt];
                ModifierKeyState[EModifierKey::RightAlt] = true;
            }
            break;
        case VK_CONTROL:
            // Differentiate between left and right control
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LCONTROL;
                bIsRepeat = ModifierKeyState[EModifierKey::LeftControl];
                ModifierKeyState[EModifierKey::LeftControl] = true;
            }
            else
            {
                ActualKey = VK_RCONTROL;
                bIsRepeat = ModifierKeyState[EModifierKey::RightControl];
                ModifierKeyState[EModifierKey::RightControl] = true;
            }
            break;
        case VK_SHIFT:
            // Differentiate between left and right shift
            ActualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            if (ActualKey == VK_LSHIFT)
            {
                bIsRepeat = ModifierKeyState[EModifierKey::LeftShift];
                ModifierKeyState[EModifierKey::LeftShift] = true;
            }
            else
            {
                bIsRepeat = ModifierKeyState[EModifierKey::RightShift];
                ModifierKeyState[EModifierKey::RightShift] = true;
            }
            break;
        case VK_CAPITAL:
            ModifierKeyState[EModifierKey::CapsLock] = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            break;
        default:
            // No translation needed
            break;
        }

        const uint32 CharCode = ::MapVirtualKey(Win32Key, MAPVK_VK_TO_CHAR);
        OnKeyDown(ActualKey, CharCode, bIsRepeat);
        return;
    }

    case WM_SYSKEYUP:      // 시스템 키(Alt, F10 등)가 눌렸다가 떼어질 때 발생하는 메시지
    case WM_KEYUP:         // 키보드 키가 떼어졌을 때 발생하는 메시지
    {
        // Character code is stored in WPARAM
        const int32 Win32Key = static_cast<int32>(wParam);
        int32 ActualKey = Win32Key;

        switch (Win32Key)
        {
        case VK_MENU:
            // Differentiate between left and right alt
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LMENU;
                ModifierKeyState[EModifierKey::LeftAlt] = false;
            }
            else
            {
                ActualKey = VK_RMENU;
                ModifierKeyState[EModifierKey::RightAlt] = false;
            }
            break;
        case VK_CONTROL:
            // Differentiate between left and right control
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LCONTROL;
                ModifierKeyState[EModifierKey::LeftControl] = false;
            }
            else
            {
                ActualKey = VK_RCONTROL;
                ModifierKeyState[EModifierKey::RightControl] = false;
            }
            break;
        case VK_SHIFT:
            // Differentiate between left and right shift
            ActualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            if (ActualKey == VK_LSHIFT)
            {
                ModifierKeyState[EModifierKey::LeftShift] = false;
            }
            else
            {
                ModifierKeyState[EModifierKey::RightShift] = false;
            }
            break;
        case VK_CAPITAL:
            ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            break;
        default:
            // No translation needed
            break;
        }

        // Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
        const uint32 CharCode = ::MapVirtualKey(Win32Key, MAPVK_VK_TO_CHAR);

        // Key up events are never repeats
        constexpr bool bIsRepeat = false;
        OnKeyUp(ActualKey, CharCode, bIsRepeat);
        return;
    }

    // Mouse Button Down
    case WM_LBUTTONDBLCLK: // 마우스 왼쪽 버튼이 더블 클릭되었을 때 발생하는 메시지
    case WM_LBUTTONDOWN:   // 마우스 왼쪽 버튼이 눌렸을 때 발생하는 메시지
    case WM_MBUTTONDBLCLK: // 마우스 가운데 버튼이 더블 클릭되었을 때 발생하는 메시지
    case WM_MBUTTONDOWN:   // 마우스 가운데 버튼이 눌렸을 때 발생하는 메시지
    case WM_RBUTTONDBLCLK: // 마우스 오른쪽 버튼이 더블 클릭되었을 때 발생하는 메시지
    case WM_RBUTTONDOWN:   // 마우스 오른쪽 버튼이 눌렸을 때 발생하는 메시지
    case WM_XBUTTONDBLCLK: // 마우스 추가 버튼(X1, X2)이 더블 클릭되었을 때 발생하는 메시지
    case WM_XBUTTONDOWN:   // 마우스 추가 버튼(X1, X2)이 눌렸을 때 발생하는 메시지
    case WM_XBUTTONUP:     // 마우스 추가 버튼(X1, X2)이 떼어졌을 때 발생하는 메시지
    case WM_LBUTTONUP:     // 마우스 왼쪽 버튼이 떼어졌을 때 발생하는 메시지
    case WM_MBUTTONUP:     // 마우스 가운데 버튼이 떼어졌을 때 발생하는 메시지
    case WM_RBUTTONUP:     // 마우스 오른쪽 버튼이 떼어졌을 때 발생하는 메시지
    {
        POINT CursorPoint;
        CursorPoint.x = GET_X_LPARAM(lParam);
        CursorPoint.y = GET_Y_LPARAM(lParam);

        ClientToScreen(hWnd, &CursorPoint);

        const FVector2D CursorPos{
            static_cast<float>(CursorPoint.x),
            static_cast<float>(CursorPoint.y)
        };

        EMouseButtons::Type MouseButton = EMouseButtons::Invalid;
        bool bDoubleClick = false;
        bool bMouseUp = false;
        switch (Msg)
        {
        case WM_LBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = EMouseButtons::Left;
            break;
        case WM_LBUTTONUP:
            bMouseUp = true;
            MouseButton = EMouseButtons::Left;
            break;
        case WM_LBUTTONDOWN:
            MouseButton = EMouseButtons::Left;
            break;
        case WM_MBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = EMouseButtons::Middle;
            break;
        case WM_MBUTTONUP:
            bMouseUp = true;
            MouseButton = EMouseButtons::Middle;
            break;
        case WM_MBUTTONDOWN:
            MouseButton = EMouseButtons::Middle;
            break;
        case WM_RBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = EMouseButtons::Right;
            break;
        case WM_RBUTTONUP:
            bMouseUp = true;
            MouseButton = EMouseButtons::Right;
            break;
        case WM_RBUTTONDOWN:
            MouseButton = EMouseButtons::Right;
            break;
        case WM_XBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
            break;
        case WM_XBUTTONUP:
            bMouseUp = true;
            MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
            break;
        case WM_XBUTTONDOWN:
            MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
            break;
        default:
            assert(0);
        }

        if (bMouseUp)
        {
            OnMouseUp(MouseButton, CursorPos);
        }
        else if (bDoubleClick)
        {
            OnMouseDoubleClick(MouseButton, CursorPos);
        }
        else
        {
            OnMouseDown(MouseButton, CursorPos);
        }
        return;
    }

    // Mouse Movement
    case WM_INPUT:
    case WM_NCMOUSEMOVE: // 비클라이언트 영역(창 제목 표시줄 등)에서 마우스가 움직였을 때 발생하는 메시지
    case WM_MOUSEMOVE:   // 클라이언트 영역에서 마우스가 움직였을 때 발생하는 메시지
    {
        OnMouseMove(); // TODO: UE [WindowsApplication.cpp:2286]
        return;
    }

    // 마우스 휠이 움직였을 때 발생하는 메시지 (올리면 +120, 내리면 -120)
    case WM_MOUSEWHEEL:
    {
        constexpr float SpinFactor = 1 / 120.0f;
        const SHORT WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        POINT CursorPoint;
        CursorPoint.x = GET_X_LPARAM(lParam);
        CursorPoint.y = GET_Y_LPARAM(lParam); 

        const FVector2D CursorPos{
            static_cast<float>(CursorPoint.x),
            static_cast<float>(CursorPoint.y)
        };
        OnMouseWheel( static_cast<float>( WheelDelta ) * SpinFactor, CursorPos );
        return;
    }
    default:
    {
        // 추후에 추가 Message가 필요하면 다음 파일을 참조
        // UnrealEngine [WindowsApplication.cpp:1990]
        return;
    }
    }
}

void FSlateAppMessageHandler::OnKeyChar(const TCHAR Character, const bool IsRepeat) const
{
    UE_LOG(LogLevel::Warning, "%c, %s", Character, IsRepeat ? "Repeat" : "");
}

void FSlateAppMessageHandler::OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) const
{
    UE_LOG(LogLevel::Warning, "%d, %c, %s", KeyCode, CharacterCode, IsRepeat ? "Repeat" : "");
}

void FSlateAppMessageHandler::OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) const
{
    UE_LOG(LogLevel::Warning, "%d, %c, %s", KeyCode, CharacterCode, IsRepeat ? "Repeat" : "");
}

void FSlateAppMessageHandler::OnMouseDown(const EMouseButtons::Type Button, const FVector2D CursorPos) const
{
    switch (Button)
    {
    case EMouseButtons::Left:
        UE_LOG(LogLevel::Warning, "Left Mouse Button Down at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Middle:
        UE_LOG(LogLevel::Warning, "Middle Mouse Button Down at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Right:
        UE_LOG(LogLevel::Warning, "Right Mouse Button Down at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Thumb01:
        UE_LOG(LogLevel::Warning, "Thumb01 Mouse Button Down at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Thumb02:
        UE_LOG(LogLevel::Warning, "Thumb02 Mouse Button Down at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    default:
        break;
    }
}

void FSlateAppMessageHandler::OnMouseUp(const EMouseButtons::Type Button, const FVector2D CursorPos) const
{
    switch (Button)
    {
    case EMouseButtons::Left:
        UE_LOG(LogLevel::Warning, "Left Mouse Button Up at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Middle:
        UE_LOG(LogLevel::Warning, "Middle Mouse Button Up at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Right:
        UE_LOG(LogLevel::Warning, "Right Mouse Button Up at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Thumb01:
        UE_LOG(LogLevel::Warning, "Thumb01 Mouse Button Up at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Thumb02:
        UE_LOG(LogLevel::Warning, "Thumb02 Mouse Button Up at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    default:
        break;
    }
}

void FSlateAppMessageHandler::OnMouseDoubleClick(const EMouseButtons::Type Button, const FVector2D CursorPos) const
{
    switch (Button)
    {
    case EMouseButtons::Left:
        UE_LOG(LogLevel::Warning, "Left Mouse Button Double Click at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Middle:
        UE_LOG(LogLevel::Warning, "Middle Mouse Button Double Click at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Right:
        UE_LOG(LogLevel::Warning, "Right Mouse Button Double Click at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Thumb01:
        UE_LOG(LogLevel::Warning, "Thumb01 Mouse Button Double Click at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    case EMouseButtons::Thumb02:
        UE_LOG(LogLevel::Warning, "Thumb02 Mouse Button Double Click at (%f, %f)", CursorPos.X, CursorPos.Y);
        break;
    default:
        break;
    }
}

void FSlateAppMessageHandler::OnMouseWheel(const float Delta, const FVector2D CursorPos) const
{
    UE_LOG(LogLevel::Warning, "Mouse Wheel Scrolled %f at (%f, %f)", Delta, CursorPos.X, CursorPos.Y);
}

void FSlateAppMessageHandler::OnMouseMove() const
{
    UE_LOG(LogLevel::Warning, "Mouse Moved");
}

FVector2D FSlateAppMessageHandler::GetCursorPos() const
{
    POINT CursorPos;
    ::GetCursorPos(&CursorPos);

    return FVector2D{static_cast<float>(CursorPos.x), static_cast<float>(CursorPos.y)};
}

FVector2D FSlateAppMessageHandler::GetLastCursorPos() const
{
    return PreviousPosition;
}
