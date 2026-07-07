#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_type.hpp"
#include "runtime/native_string.hpp"

class IScriptTypeResolver
{
public:
    virtual Type* Resolve(std::string assemblyQualifiedTypeName) = 0;
};
