#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"
#include "RuntimeCodeModuleLoadState.hpp"
#include "runtime/array.hpp"

class RuntimeCodeModuleManifestEntry
{
public:
    virtual ~RuntimeCodeModuleManifestEntry() = default;

    std::string ModuleId;

    const std::string& get_ModuleId();

    std::string RuntimeSpecializationId;

    const std::string& get_RuntimeSpecializationId();

    ::RuntimeCodeModuleLoadState LoadState;

    ::RuntimeCodeModuleLoadState get_LoadState();

    Array<std::string>* DependencyModuleIds;

    Array<std::string>* get_DependencyModuleIds();

    RuntimeCodeModuleManifestEntry(std::string moduleId, std::string runtimeSpecializationId, ::RuntimeCodeModuleLoadState loadState, Array<std::string>* dependencyModuleIds);
};
