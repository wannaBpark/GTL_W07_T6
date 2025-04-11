#include "Vector4.h"

#include "Misc/Parse.h"

FString FVector4::ToString() const
{
    // FString::Printf를 사용하여 포맷팅된 문자열 생성
    // TEXT() 매크로는 리터럴 문자열을 TCHAR 타입으로 만들어줍니다.
	return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f W=%3.3f"), X, Y, Z, W);

    // 필요에 따라 소수점 정밀도 지정 가능: 예) "X=%.2f Y=%.2f Z=%.2f"
    // return FString::Printf(TEXT("X=%.2f Y=%.2f Z=%.2f"), x, y, z);
}

bool FVector4::InitFromString(const FString& InSourceString)
{
    X = Y = Z = 0;
    W = 1.0f;

    // The initialization is only successful if the X, Y, and Z values can all be parsed from the string
    const bool bSuccessful = FParse::Value(*InSourceString, TEXT("X=") , X) &&
            FParse::Value(*InSourceString, TEXT("Y="), Y)   &&
            FParse::Value(*InSourceString, TEXT("Z="), Z);

    // W is optional, so don't factor in its presence (or lack thereof) in determining initialization success
    FParse::Value(*InSourceString, TEXT("W="), W);

    return bSuccessful;
//             // FString에서 C-스타일 문자열 포인터 얻기 (메서드 이름은 실제 FString 구현에 맞게 조정)
//         const char* currentPos = *SourceString; // 또는 SourceString.c_str();
//         if (currentPos == nullptr) return false; // 빈 문자열 또는 오류
//
//         float parsedX, parsedY, parsedZ, parsedA;
//         char* endPtrX = nullptr;
//         char* endPtrY = nullptr;
//         char* endPtrZ = nullptr;
//         char* endPtrA = nullptr;
//
//         // 1. "X=" 찾기 및 값 파싱
//         const char* xMarker = strstr(currentPos, "X=");
//         if (xMarker == nullptr) return false; // "X=" 마커 없음
//
//         // "X=" 다음 위치로 이동
//         const char* xValueStart = xMarker + 2; // "X=" 길이만큼 이동
//
//         // 숫자 변환 시도 (strtof는 선행 공백 무시)
//         parsedX = strtof(xValueStart, &endPtrX);
//         // 변환 실패 확인 (숫자를 전혀 읽지 못함)
//         if (endPtrX == xValueStart) return false;
//
//         // 파싱 성공, 다음 검색 시작 위치 업데이트
//         currentPos = endPtrX;
//
//         // 2. "Y=" 찾기 및 값 파싱 (X 이후부터 검색)
//         const char* yMarker = strstr(currentPos, "Y=");
//         if (yMarker == nullptr) return false; // "Y=" 마커 없음
//
//         const char* yValueStart = yMarker + 2;
//         parsedY = strtof(yValueStart, &endPtrY);
//         if (endPtrY == yValueStart) return false; // 변환 실패
//
//         // 다음 검색 시작 위치 업데이트
//         currentPos = endPtrY;
//
//         // 3. "Z=" 찾기 및 값 파싱 (Y 이후부터 검색)
//         const char* zMarker = strstr(currentPos, "Z=");
//         if (zMarker == nullptr) return false; // "Z=" 마커 없음
//
//         const char* zValueStart = zMarker + 2;
//         parsedZ = strtof(zValueStart, &endPtrZ);
//         if (endPtrZ == zValueStart) return false; // 변환 실패
//
//         
//         // 다음 검색 시작 위치 업데이트
//         currentPos  = endPtrZ;
//         
//         // 4. "A=" 찾기 및 값 파싱 (Z 이후부터 검색)
//         const char* aMarker = strstr(currentPos, "A=");
//         if (aMarker == nullptr) return false; // "Z=" 마커 없음
//
//         const char* aValueStart = aMarker + 2;
//         parsedA = strtof(aValueStart, &endPtrA);
//         if (endPtrA == aValueStart) return false; // 변환 실패
//         
//
//         // 모든 파싱 성공 시, 멤버 변수 업데이트
//         X = parsedX;
//         Y = parsedY;
//         Z = parsedZ;
//         W = parsedA;
//         return true;
}
