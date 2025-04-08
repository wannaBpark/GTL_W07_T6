#pragma once
#include <assert.h>
#include "HAL/PlatformType.h"

class UObject;
class FName;


#pragma region Legacy FArchive
// class FArchive
// {
// public:
//     FArchive() = default;
//     virtual ~FArchive() = default;
//
//     FArchive(const FArchive&) = default;
//     FArchive& operator=(const FArchive& ArchiveToCopy) = default;
//     FArchive(FArchive&&) = default;
//     FArchive& operator=(FArchive&&) = default;
//
// public:
//     virtual void Serialize(void* V, int64 Length)
//     {
//     }
//
// public:
//     virtual FArchive& operator<<(FName& Value)
//     {
//         return *this;
//     }
//
//     virtual FArchive& operator<<(UObject*& Value)
//     {
//         return *this;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, ANSICHAR& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint8& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int8& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint16& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int16& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint32& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int32& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint64& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int64& Value)
//     {
//         Ar.Serialize(&Value, 1);
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, bool& D)
//     {
//         uint32 OldUBoolValue = D ? 1 : 0;
//         Ar.Serialize(&OldUBoolValue, sizeof(OldUBoolValue));
//
//         assert(OldUBoolValue == 1);
//         D = !!OldUBoolValue;
//         return Ar;
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, float& Value)
//     {
//         
//     }
//
//     FORCEINLINE friend FArchive& operator<<(FArchive& Ar, double& Value)
//     {
//         
//     }
// };
#pragma endregion

class FArchive
{
public:
    virtual ~FArchive() = default;

    bool bIsLoading = false;
    bool bIsSaving = false;
    
    // 직렬화/역직렬화를 위한 연산자 오버로딩
    template<typename T>
    FArchive& operator<<(T& Data)
    {
        if (bIsSaving)
        {
            // 데이터 저장 로직
            SaveData(&Data, sizeof(T));
        }
        else if (bIsLoading)
        {
            // 데이터 로드 로직
            LoadData(&Data, sizeof(T));
        }
        return *this;
    }
    
    // 기본 데이터 타입들을 위한 특수 처리
    virtual void SaveData(const void* Data, int64 Length) = 0;
    virtual void LoadData(void* Data, int64 Length) = 0;
};
