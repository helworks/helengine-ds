#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeCodeModuleManifestEntry.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeCodeModuleLoadState.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeCodeModuleManifestEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& RuntimeCodeModuleManifestEntry::get_ModuleId()
{
return this->ModuleId;
}

const std::string& RuntimeCodeModuleManifestEntry::get_RuntimeSpecializationId()
{
return this->RuntimeSpecializationId;
}

::RuntimeCodeModuleLoadState RuntimeCodeModuleManifestEntry::get_LoadState()
{
return this->LoadState;
}

Array<std::string>* RuntimeCodeModuleManifestEntry::get_DependencyModuleIds()
{
return this->DependencyModuleIds;
}

RuntimeCodeModuleManifestEntry::RuntimeCodeModuleManifestEntry(std::string moduleId, std::string runtimeSpecializationId, ::RuntimeCodeModuleLoadState loadState, Array<std::string>* dependencyModuleIds) : ModuleId(), RuntimeSpecializationId(), LoadState(), DependencyModuleIds()
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_00000127 = "Runtime code module id is required.";
auto __ctor_arg_00000128 = "moduleId";
return new ArgumentException(__ctor_arg_00000127, __ctor_arg_00000128);
})();
    }
    if (String::IsNullOrWhiteSpace(runtimeSpecializationId))
    {
throw ([&]() {
auto __ctor_arg_00000129 = "Runtime code module specialization id is required.";
auto __ctor_arg_0000012A = "runtimeSpecializationId";
return new ArgumentException(__ctor_arg_00000129, __ctor_arg_0000012A);
})();
    }
    if (dependencyModuleIds == nullptr)
    {
throw new ArgumentNullException("dependencyModuleIds");
    }
for (int32_t index = 0; index < dependencyModuleIds->get_Length(); index++) {
const std::string dependencyModuleId = (*dependencyModuleIds)[index];
    if (String::IsNullOrWhiteSpace(dependencyModuleId))
    {
throw ([&]() {
auto __ctor_arg_0000012B = "Runtime code module dependencies cannot contain blank entries.";
auto __ctor_arg_0000012C = "dependencyModuleIds";
return new ArgumentException(__ctor_arg_0000012B, __ctor_arg_0000012C);
})();
    }
}
this->ModuleId = moduleId;
this->RuntimeSpecializationId = runtimeSpecializationId;
this->LoadState = loadState;
this->DependencyModuleIds = dependencyModuleIds;
}

