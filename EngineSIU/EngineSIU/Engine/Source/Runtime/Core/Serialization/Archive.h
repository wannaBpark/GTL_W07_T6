#pragma once
#include <assert.h>
#include "HAL/PlatformType.h"

class UObject;
class FName;


class FArchive
{
public:
    FArchive() = default;
    virtual ~FArchive() = default;

    FArchive(const FArchive&) = default;
    FArchive& operator=(const FArchive& ArchiveToCopy) = default;
    FArchive(FArchive&&) = default;
    FArchive& operator=(FArchive&&) = default;

public:
    virtual void Serialize(void* V, int64 Length)
    {
    }

public:
    virtual FArchive& operator<<(FName& Value)
    {
        return *this;
    }

    virtual FArchive& operator<<(UObject*& Value)
    {
        return *this;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, ANSICHAR& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }


    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint8& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int8& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint16& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int16& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint32& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int32& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, uint64& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, int64& Value)
    {
        Ar.Serialize(&Value, 1);
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, bool& D)
    {
        uint32 OldUBoolValue = D ? 1 : 0;
        Ar.Serialize(&OldUBoolValue, sizeof(OldUBoolValue));

        assert(OldUBoolValue == 1);
        D = !!OldUBoolValue;
        return Ar;
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, float& Value)
    {
        
    }

    FORCEINLINE friend FArchive& operator<<(FArchive& Ar, double& Value)
    {
        
    }
};
