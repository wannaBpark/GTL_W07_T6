#include "Color.h"

const FLinearColor FLinearColor::White(1.f, 1.f, 1.f);
const FLinearColor FLinearColor::Gray(0.5f, 0.5f, 0.5f);
const FLinearColor FLinearColor::Black(0, 0, 0);
const FLinearColor FLinearColor::Transparent(0, 0, 0, 0);
const FLinearColor FLinearColor::Red(1.f, 0, 0);
const FLinearColor FLinearColor::Green(0, 1.f, 0);
const FLinearColor FLinearColor::Blue(0, 0, 1.f);
const FLinearColor FLinearColor::Yellow(1.f, 1.f, 0);

const FColor FColor::White(255, 255, 255);
const FColor FColor::Black(0, 0, 0);
const FColor FColor::Transparent(0, 0, 0, 0);
const FColor FColor::Red(255, 0, 0);
const FColor FColor::Green(0, 255, 0);
const FColor FColor::Blue(0, 0, 255);
const FColor FColor::Yellow(255, 255, 0);
const FColor FColor::Cyan(0, 255, 255);
const FColor FColor::Magenta(255, 0, 255);
const FColor FColor::Orange(243, 156, 18);
const FColor FColor::Purple(169, 7, 228);
const FColor FColor::Turquoise(26, 188, 156);
const FColor FColor::Silver(189, 195, 199);
const FColor FColor::Emerald(46, 204, 113);

FString FLinearColor::ToString() const
{
      return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), R, G, B, A);
}

bool FLinearColor::InitFromString(const FString& SourceString)
{
    const char* currentPos = *SourceString;
    if (currentPos == nullptr) return false;

    float parsedR, parsedG, parsedB, parsedA; // 변수 이름 변경 (X,Y,Z -> R,G,B)
    char* endPtrR = nullptr;
    char* endPtrG = nullptr;
    char* endPtrB = nullptr;
    char* endPtrA = nullptr;

    // --- R, G, B, A 파싱 로직 (InitFromString을 수정한다면) ---
    // 예시: R 파싱
    const char* rMarker = strstr(currentPos, "R="); // "R=" 찾기
    if (rMarker == nullptr) return false;
    const char* rValueStart = rMarker + 2;
    parsedR = strtof(rValueStart, &endPtrR);
    if (endPtrR == rValueStart) return false;
    currentPos = endPtrR;

    // 예시: G 파싱
    const char* gMarker = strstr(currentPos, "G="); // "G=" 찾기
    if (gMarker == nullptr) return false;
    const char* gValueStart = gMarker + 2;
    parsedG = strtof(gValueStart, &endPtrG);
    if (endPtrG == gValueStart) return false;
    currentPos = endPtrG;

    // 예시: B 파싱
    const char* bMarker = strstr(currentPos, "B="); // "B=" 찾기
    if (bMarker == nullptr) return false;
    const char* bValueStart = bMarker + 2;
    parsedB = strtof(bValueStart, &endPtrB);
    if (endPtrB == bValueStart) return false;
    currentPos = endPtrB;

    // 예시: A 파싱
    const char* aMarker = strstr(currentPos, "A="); // "A=" 찾기
    if (aMarker == nullptr) return false;
    const char* aValueStart = aMarker + 2;
    parsedA = strtof(aValueStart, &endPtrA);
    if (endPtrA == aValueStart) return false;

    // 모든 파싱 성공 시, 멤버 변수 업데이트
    R = parsedR;
    G = parsedG;
    B = parsedB;
    A = parsedA;
    return true;

}
