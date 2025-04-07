// ReSharper disable CppClangTidyBugproneMacroParentheses
#pragma once
#include "Class.h"

// name을 문자열화 해주는 매크로
#define INLINE_STRINGIFY(name) #name


// RTTI를 위한 클래스 매크로
#define DECLARE_CLASS(TClass, TSuperClass) \
private: \
    TClass(const TClass&) = delete; \
    TClass& operator=(const TClass&) = delete; \
    TClass(TClass&&) = delete; \
    TClass& operator=(TClass&&) = delete; \
public: \
    using Super = TSuperClass; \
    using ThisClass = TClass; \
    static UClass* StaticClass() { \
        static UClass ClassInfo{ \
            TEXT(#TClass), \
            static_cast<uint32>(sizeof(TClass)), \
            static_cast<uint32>(alignof(TClass)), \
            TSuperClass::StaticClass(), \
            []() -> UObject* \
            { \
                void* RawMemory = FPlatformMemory::Malloc<EAT_Object>(sizeof(TClass)); \
                ::new (RawMemory) TClass; \
                return static_cast<UObject*>(RawMemory); \
            } \
        }; \
        return &ClassInfo; \
    }

// RTTI를 위한 클래스 매크로
#define DECLARE_ABSTRACT_CLASS(TClass, TSuperClass) \
private: \
    TClass(const TClass&) = delete; \
    TClass& operator=(const TClass&) = delete; \
    TClass(TClass&&) = delete; \
    TClass& operator=(TClass&&) = delete; \
public: \
    using Super = TSuperClass; \
    using ThisClass = TClass; \
    static UClass* StaticClass() { \
        static UClass ClassInfo{ \
            TEXT(#TClass), \
            static_cast<uint32>(sizeof(TClass)), \
            static_cast<uint32>(alignof(TClass)), \
            TSuperClass::StaticClass(), \
            []() -> UObject* { return nullptr; } \
        }; \
        return &ClassInfo; \
    }


/**
 * UClass에 Property를 등록합니다.
 * @param Type 선언할 타입
 * @param VarName 변수 이름
 * @param DefaultExpr 기본값
 *
 * Example Code
 * ```
 * UPROPERTY
 * (int, Value, = 10)
 * ```
 */
#define UPROPERTY(Type, VarName, DefaultExpr) \
    Type VarName DefaultExpr; \
    inline static struct VarName##_PropRegistrar \
    { \
        VarName##_PropRegistrar() \
        { \
            constexpr size_t Offset = offsetof(ThisClass, VarName); \
            ThisClass::StaticClass()->RegisterProperty( \
                { #VarName, sizeof(Type), Offset } \
            ); \
        } \
    } VarName##_PropRegistrar_{};
