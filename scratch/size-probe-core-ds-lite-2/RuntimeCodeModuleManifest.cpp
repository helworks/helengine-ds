#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeCodeModuleManifest.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeCodeModuleLoadState.hpp"
#include "RuntimeCodeModuleManifestEntry.hpp"
#include "RuntimeCodeModuleManifest.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<::RuntimeCodeModuleManifestEntry*>* RuntimeCodeModuleManifest::get_Entries()
{
return this->Entries;
}

bool RuntimeCodeModuleManifest::CanUnloadModule(std::string moduleId)
{
return this->GetLoadState(moduleId) != RuntimeCodeModuleLoadState::ResidentAtStartup;}

Array<std::string>* RuntimeCodeModuleManifest::GetResidentModuleIds()
{
Array<std::string> *residentModuleIds = new Array<std::string>(this->Entries->get_Length());
int32_t residentModuleCount = 0;
for (int32_t index = 0; index < this->Entries->get_Length(); index++) {
::RuntimeCodeModuleManifestEntry *entry = (*this->Entries)[index];
    if (entry->LoadState != RuntimeCodeModuleLoadState::Unloadable)
    {
(*residentModuleIds)[residentModuleCount] = entry->ModuleId;
residentModuleCount++;
    }
}
    if (residentModuleCount == residentModuleIds->get_Length())
    {
return residentModuleIds;    }
Array<std::string> *exactResidentModuleIds = new Array<std::string>(residentModuleCount);
for (int32_t index = 0; index < residentModuleCount; index++) {
(*exactResidentModuleIds)[index] = (*residentModuleIds)[index];
}
return exactResidentModuleIds;}

Array<std::string>* RuntimeCodeModuleManifest::GetUnloadableModuleIds()
{
Array<std::string> *unloadableModuleIds = new Array<std::string>(this->Entries->get_Length());
int32_t unloadableModuleCount = 0;
for (int32_t index = 0; index < this->Entries->get_Length(); index++) {
::RuntimeCodeModuleManifestEntry *entry = (*this->Entries)[index];
    if (entry->LoadState == RuntimeCodeModuleLoadState::Unloadable)
    {
(*unloadableModuleIds)[unloadableModuleCount] = entry->ModuleId;
unloadableModuleCount++;
    }
}
    if (unloadableModuleCount == unloadableModuleIds->get_Length())
    {
return unloadableModuleIds;    }
Array<std::string> *exactUnloadableModuleIds = new Array<std::string>(unloadableModuleCount);
for (int32_t index = 0; index < unloadableModuleCount; index++) {
(*exactUnloadableModuleIds)[index] = (*unloadableModuleIds)[index];
}
return exactUnloadableModuleIds;}

RuntimeCodeModuleManifest::RuntimeCodeModuleManifest(Array<::RuntimeCodeModuleManifestEntry*>* entries) : Entries()
{
    if (entries == nullptr)
    {
throw new ArgumentNullException("entries");
    }
this->Entries = entries;
}

::RuntimeCodeModuleLoadState RuntimeCodeModuleManifest::GetLoadState(std::string moduleId)
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_00000125 = "Module id is required.";
auto __ctor_arg_00000126 = "moduleId";
return new ArgumentException(__ctor_arg_00000125, __ctor_arg_00000126);
})();
    }
for (int32_t index = 0; index < this->Entries->get_Length(); index++) {
::RuntimeCodeModuleManifestEntry *entry = (*this->Entries)[index];
    if (String::Equals(entry->ModuleId, moduleId, StringComparison::OrdinalIgnoreCase))
    {
return entry->LoadState;    }
}
throw new InvalidOperationException(std::string("Runtime code module '") + moduleId + std::string("' was not found in the residency manifest."));
}

