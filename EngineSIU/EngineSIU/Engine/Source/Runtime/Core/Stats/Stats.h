#pragma once
#include "Container/String.h"
#include "HAL/PlatformType.h"
#include "UObject/NameTypes.h"


struct TStatId
{
    FName Name;

    TStatId()
        : Name(NAME_None)
    {
    }

    TStatId(FName Name)
        : Name(Name)
    {
    }

    FName GetName() const
    {
        return Name;
    }

    bool operator==(const TStatId& Other) const
    {
        return Name == Other.Name;
    }

    bool operator!=(const TStatId& Other) const
    {
        return Name != Other.Name;
    }
};


class FScopeCycleCounter
{
public:
    FScopeCycleCounter(TStatId StatId);
    ~FScopeCycleCounter();

    // 이동 & 복사 생성자 제거
    FScopeCycleCounter(const FScopeCycleCounter&) = delete;
    FScopeCycleCounter& operator=(const FScopeCycleCounter&) = delete;
    FScopeCycleCounter(FScopeCycleCounter&&) = delete;
    FScopeCycleCounter& operator=(FScopeCycleCounter&&) = delete;

    uint64 Finish();

private:
    uint64 StartCycles;

    [[maybe_unused]]
    TStatId UsedStatId;
};

#define QUICK_SCOPE_CYCLE_COUNTER(Stat) \
    static TStatId FStat_##Stat(TEXT(#Stat)); \
    FScopeCycleCounter CycleCount_##Stat(FStat_##Stat);
