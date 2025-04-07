#pragma once
#include "Object.h"


struct FProperty
{
    FProperty(const char* InName, int32 InSize, int32 InOffset)
        : PropertyName(InName)
        , PropertySize(InSize)
        , PropertyOffset(InOffset)
    {}

    virtual ~FProperty() = default;

    const char* PropertyName;
    size_t PropertySize;
    size_t PropertyOffset;

    // virtual void Serialize(void* Data) const = 0;
};


// struct FNumericProperty : public FProperty
// {
//     FNumericProperty(const char* InName, int32 InSize, int32 InOffset)
//         : FProperty(InName, InSize, InOffset)
//     {}
// };
//
// struct FInt8Property : public FNumericProperty
// {
//     FInt8Property(const char* InName, int32 InSize, int32 InOffset)
//         : FNumericProperty(InName, InSize, InOffset)
//     {}
//
//     virtual void Serialize(void* Data) const override
//     {
//         
//     }
// };