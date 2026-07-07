#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

class PersistedComponentTypeResolver
{
public:
    virtual ~PersistedComponentTypeResolver() = default;

    static Type* TryResolve(std::string componentTypeId);
private:
    static Array<std::string>* CandidateAssemblyNames;

    static Type* TryResolveAssemblyQualifiedType(std::string componentTypeId);
};
