#pragma once
#include "Archive.h"
#include "Container/Array.h"


class FMemoryWriter : public FArchive
{
private:
    TArray<uint8> Data;

public:
    FMemoryWriter()
    {
        bIsSaving = true;
        bIsLoading = false;
    }

    virtual void SaveData(const void* InData, int64 Length) override
    {
        const int32 Index = Data.Num();
        Data.AddUninitialized(static_cast<int32>(Length));
        FPlatformMemory::Memcpy(Data.GetData() + Index, InData, Length);
    }

    TArray<uint8>& GetData() { return Data; }
};

class FMemoryReader : public FArchive
{
private:
    TArray<uint8> Data;
    int64 Position = 0;

public:
    FMemoryReader(const TArray<uint8>& InData)
        : Data(InData)
    {
        bIsSaving = false;
        bIsLoading = true;
    }

    virtual void LoadData(void* OutData, int64 Length) override
    {
        FPlatformMemory::Memcpy(OutData, Data.GetData() + Position, Length);
        Position += Length;
    }
};
