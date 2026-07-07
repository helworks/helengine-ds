#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_type.hpp"

class ScenePersistenceDictionaryTypeSupport
{
public:
    virtual ~ScenePersistenceDictionaryTypeSupport() = default;

    static int32_t CompareKeys(void* leftKey, void* rightKey, Type* keyType);

    static bool IsDictionaryType__out1_out2(Type* valueType, Type*& keyType, Type*& dictionaryValueType);

    static bool IsSupportedDictionaryKeyType(Type* keyType);
private:
    static HashSet<Type>* SupportedScalarKeyTypes;
};
