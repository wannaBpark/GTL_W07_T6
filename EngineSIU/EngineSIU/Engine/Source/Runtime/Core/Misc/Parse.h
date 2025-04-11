#pragma once
#include "HAL/PlatformType.h"
//#include "GameFramework/Actor.h"

class FName;
class FString;

struct FParse
{
//     // 재귀적으로 문자열을 빌드하는 템플릿 함수 (ToString 용)
//     template<typename T>
//     void AppendToStringRecursive(FString& OutStr, const T& obj)
//     {
//         // 재귀 종료 조건 (더 이상 처리할 인자가 없음)
//     }
//
//     // --- AppendToStringRecursive 수정 ---
//     template<typename T, typename MemberType, typename... Args>
//     void AppendToStringRecursive(FString& OutStr, const T& obj, const char* Name, MemberType T::* MemberPtr, Args... args)
//     {
//         if (!OutStr.IsEmpty()) { OutStr += TEXT(" "); }
//
//         // C++17 if constexpr 사용
//         if constexpr (std::is_floating_point_v<MemberType>) { // float 또는 double 인 경우
//             OutStr += FString::Printf(TEXT("%s=%3.3f"), Name, obj.*MemberPtr);
//         } else if constexpr (std::is_integral_v<MemberType>) { // int, long, short, bool 등 정수 계열
//             // bool은 "true"/"false"로 출력하는 것이 좋을 수 있음
//             if constexpr (std::is_same_v<MemberType, bool>) {
//                 OutStr += FString::Printf(TEXT("%s=%s"), Name, (obj.*MemberPtr ? TEXT("true") : TEXT("false")));
//             } else {
//                 // %lld는 long long, %d는 int 등 타입에 맞게 조정 필요
//                 // FString::FromInt 같은 범용 함수 사용 권장
//                 OutStr += FString::Printf(TEXT("%s=%lld"), Name, static_cast<long long>(obj.*MemberPtr));
//             }
//         } else if constexpr (std::is_same_v<MemberType, FString>) { // FString 타입인 경우
//             // FString은 따옴표로 감싸는 것이 좋을 수 있음 (선택적)
//             OutStr += FString::Printf(TEXT("%s=\"%s\""), Name, *(obj.*MemberPtr));
//         }
//         // ... 다른 타입들에 대한 처리 추가 (FName, Enum 등) ...
//         else {
//             // 지원하지 않는 타입 처리 (컴파일 에러 유도 또는 기본 문자열)
//             static_assert(false, "Unsupported type for AppendToStringRecursive");
//             //OutStr += FString::Printf(TEXT("%s=<unsupported>"), Name);
//         }
//
//
//         AppendToStringRecursive(OutStr, obj, args...);
//     }
//
//     // 재귀적으로 문자열을 파싱하는 템플릿 함수 (InitFromString 용)
//     template<typename T>
//     bool ParseFromStringRecursive(const char*& currentPos, T& obj)
//     {
//         // 재귀 종료 조건 (모든 인자 처리 완료)
//         return true;
//     }
//
//     template<typename T, typename MemberType, typename... Args>
//     bool ParseFromStringRecursive(const char*& currentPos, T& obj, const char* Name, MemberType T::* MemberPtr, Args... args)
//     {
//         // 마커 ("Name=") 찾기
//         FString Marker = FString::Printf(TEXT("%s="), Name); // TCHAR 가정
//         const char* foundMarker = strstr(currentPos, *Marker);
//         if (!foundMarker) return false; // 마커 못 찾으면 실패
//
//         // 값 시작 위치 계산 및 float 파싱
//         const char* valueStart = foundMarker + Marker.Len();
//         char* endPtr = nullptr;
//         float parsedValue = strtof(valueStart, &endPtr); // 표준 C 함수 사용
//         if (endPtr == valueStart) return false; // 파싱 실패
//
//         // 파싱된 값을 멤버 변수에 할당
//         obj.*MemberPtr = parsedValue;
//
//         // 다음 검색 시작 위치 업데이트
//         currentPos = endPtr;
//
//         // 나머지 인자들로 재귀 호출
//         return ParseFromStringRecursive(currentPos, obj, args...);
//     }

    /** Parses a string from a text string. 
     * @param Stream, the string you want to extract the value from.
     * @param Match, the identifier for the value in the stream.
     * @param Value, the destination to the value to be extracted to.
     * @param MaxLen, the maximum size eof the string that can be extracted.
     * @param bShouldStopOnSeparator, (default = true) If this is true, and the value doesn't start with a '"'
                then it may be truncated to ',' or ')' in addition to whitespace.
     * @param OptStreamGotTo, (default = nullptr) If this is not null, then its dereference is set to the address
                of the end of the value within Stream. This permits consuming of stream in a loop where Match may
                occur multiple times.
    */
    static bool Value( const TCHAR* Stream, const TCHAR* Match, TCHAR* Value, int32 MaxLen, bool bShouldStopOnSeparator=true, const TCHAR** OptStreamGotTo = nullptr);
	

    /** Parses a name. */
    static  bool Value( const TCHAR* Stream, const TCHAR* Match, FName& Name );
    /** Parses a uint32. */
    static  bool Value( const TCHAR* Stream, const TCHAR* Match, uint32& Value );

    static bool Value( const TCHAR* Stream, const TCHAR* Match, uint8& Value );
    /** Parses a signed byte. */
    static bool Value( const TCHAR* Stream, const TCHAR* Match, int8& Value );
    /** Parses a uint16. */
    static bool Value( const TCHAR* Stream, const TCHAR* Match, uint16& Value );
    /** Parses a signed word. */
    static bool Value( const TCHAR* Stream, const TCHAR* Match, int16& Value );
    /** Parses a floating-point value. */
    static bool Value( const TCHAR* Stream, const TCHAR* Match, float& Value );
    /** Parses a double precision floating-point value. */
    static bool Value(const TCHAR* Stream, const TCHAR* Match, double& Value);
    /** Parses a signed double word. */
    static bool Value( const TCHAR* Stream, const TCHAR* Match, int32& Value );

    /** Parses a string. 
 * @param Stream, the string you want to extract the value from.
 * @param Match, the identifier for the value in the stream.
 * @param Value, the destination to the value to be extracted to.
 * @param bShouldStopOnSeparator, (default = true) If this is true, and the value doesn't start with a '"'
            then it may be truncated to ',' or ')' in addition to whitespace.
 * @param OptStreamGotTo, (default = nullptr) If this is not null, then its dereference is set to the address
            of the end of the value within Stream. This permits consuming of stream in a loop where Match may
            occur multiple times.
*/
    //static bool Value( const TCHAR* Stream, const TCHAR* Match, FString& Value, bool bShouldStopOnSeparator =true, const TCHAR** OptStreamGotTo = nullptr);
    
    /** Parses a boolean value. */
    static bool Bool( const TCHAR* Stream, const TCHAR* Match, bool& OnOff );
    
};
