#include "EngineStatics.h"


uint32 UEngineStatics::GenUUID()
{
    UE_LOG(LogLevel::Display, "Generate UUID : %d", NextUUID);
    return NextUUID++;
}
