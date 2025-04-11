#pragma once
#include <cassert>

// 예시: 커스텀 check 매크로 정의 (보통 공통 헤더 파일에 위치)
#if defined(_DEBUG) || defined(DEBUG)
    // 디버그 빌드: 표준 assert 사용
    #define check(expr) assert(expr)
    // #define check(expr, format, ...) assert((expr) && format) // 메시지 포함 버전 (assert는 직접 포맷팅 지원 안함)
#else
    // 릴리스 빌드: 아무것도 하지 않음 (컴파일러 최적화로 제거됨)
    #define MY_CHECK(expr) ((void)0) // 컴파일러 경고 방지
    //#define MY_CHECKF(expr, format, ...) ((void)0)
#endif
