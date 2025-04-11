#include "WindowsCursor.h"


FVector2D FWindowsCursor::GetPosition()
{
    POINT CursorPos;
    ::GetCursorPos(&CursorPos);

    return {
        static_cast<float>(CursorPos.x),
        static_cast<float>(CursorPos.y)
    };
}

void FWindowsCursor::SetPosition(const int32 X, const int32 Y)
{
    ::SetCursorPos(X, Y);
}
