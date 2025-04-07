#pragma once
#include "UTextComponent.h"

class UTextUUID : public UText
{
    DECLARE_CLASS(UTextUUID, UText)

public:
    UTextUUID();

    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;
    void SetUUID(uint32 UUID);
};
