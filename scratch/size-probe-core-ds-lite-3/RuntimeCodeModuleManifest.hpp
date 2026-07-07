#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeCodeModuleManifestEntry;

#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "RuntimeCodeModuleLoadState.hpp"

class RuntimeCodeModuleManifest
{
public:
    virtual ~RuntimeCodeModuleManifest() = default;

    Array<::RuntimeCodeModuleManifestEntry*>* Entries;

    Array<::RuntimeCodeModuleManifestEntry*>* get_Entries();

    bool CanUnloadModule(std::string moduleId);

    Array<std::string>* GetResidentModuleIds();

    Array<std::string>* GetUnloadableModuleIds();

    RuntimeCodeModuleManifest(Array<::RuntimeCodeModuleManifestEntry*>* entries);
private:
    ::RuntimeCodeModuleLoadState GetLoadState(std::string moduleId);
};
