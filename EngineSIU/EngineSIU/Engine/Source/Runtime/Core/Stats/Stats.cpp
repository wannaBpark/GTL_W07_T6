#include "Stats.h"
#include "WindowsPlatformTime.h"


FScopeCycleCounter::FScopeCycleCounter(TStatId StatId)
    : StartCycles(FPlatformTime::Cycles64())
    , UsedStatId(StatId)
{
}

FScopeCycleCounter::~FScopeCycleCounter()
{
    Finish();
}

uint64 FScopeCycleCounter::Finish()
{
    const uint64 EndCycles = FPlatformTime::Cycles64();
    const uint64 CycleDiff = EndCycles - StartCycles;

    // FThreadStats::AddMessage(UsedStatId, EStatOperation::Add, CycleDiff);

    return CycleDiff;
}

