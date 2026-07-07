#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IScriptTypeResolver;

#include "IScriptTypeResolver.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

class ScriptTypeResolver : public ::IScriptTypeResolver
{
public:
    virtual ~ScriptTypeResolver() = default;

    void Clear();

    void Register(std::string moduleId, Assembly* assembly);

    Type* Resolve(std::string assemblyQualifiedTypeName);

    ScriptTypeResolver();
private:
    Dictionary<std::string, Assembly*>* AssembliesByModuleId;
};
