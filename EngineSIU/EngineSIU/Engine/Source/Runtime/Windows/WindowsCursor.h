#pragma once
#include "HAL/PlatformType.h"
#include "Math/Vector.h"


/**
 * WinAPI에서 Cursor관련 로직을 래핑한 클래스 입니다. 
 */
struct FWindowsCursor
{
    /** 현재 Cursor의 위치를 가져옵니다. */
    static FVector2D GetPosition();

    /** Cursor의 위치를 설정합니다. */
    static void SetPosition(const int32 X, const int32 Y);
};
