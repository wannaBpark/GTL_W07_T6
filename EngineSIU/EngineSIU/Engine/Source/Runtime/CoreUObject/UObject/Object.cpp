#include "Object.h"

#include "ObjectFactory.h"
#include "UClass.h"
#include "Engine/Engine.h"


UClass* UObject::StaticClass()
{
    static UClass ClassInfo{
        TEXT("UObject"),
        sizeof(UObject),
        alignof(UObject),
        nullptr,
        []() -> UObject*
        {
            void* RawMemory = FPlatformMemory::Malloc<EAT_Object>(sizeof(UObject));
            ::new (RawMemory) UObject;
            return static_cast<UObject*>(RawMemory);
        }
    };
    return &ClassInfo;
}

UObject::UObject()
    : UUID(0)
    // TODO: Object를 생성할 때 직접 설정하기
    , InternalIndex(-1)
    , NamePrivate("None")
{
}

UObject* UObject::Duplicate()
{
    return FObjectFactory::ConstructObject(GetClass());
}

UWorld* UObject::GetWorld() const
{
    return GEngine->ActiveWorld.get();
}

bool UObject::IsA(const UClass* SomeBase) const
{
    const UClass* ThisClass = GetClass();
    return ThisClass->IsChildOf(SomeBase);
}
