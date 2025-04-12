#pragma once
#include "HAL/PlatformType.h"
#include "Math/Vector.h"


enum class ECursorType : uint8
{
    None = 0,
    Arrow,
    TextEditBeam,
    Crosshairs,
    Hand,
    ResizeLeftRight,
    ResizeUpDown,
    ResizeSouthEast,
    ResizeSouthWest,
    Max
};

/**
 * WinAPI에서 Cursor관련 로직을 래핑한 클래스 입니다. 
 */
struct FWindowsCursor
{
    /** 현재 Cursor의 위치를 가져옵니다. */
    static FVector2D GetPosition();

    /** Cursor의 위치를 설정합니다. */
    static void SetPosition(const int32 X, const int32 Y);

    /** Cursor의 모양을 가져옵니다. */
    static ECursorType GetMouseCursor();

    /** Cursor의 모양을 설정합니다. */
    static void SetMouseCursor(const ECursorType CursorType = ECursorType::Arrow);
};
