#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneCatalog.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "system/string_comparer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/string_comparer.hpp"

Array<::RuntimeSceneCatalogEntry*>* RuntimeSceneCatalog::get_Entries()
{
return this->Entries;
}

RuntimeSceneCatalog::RuntimeSceneCatalog(Array<::RuntimeSceneCatalogEntry*>* entries) : Entries(), EntriesBySceneId()
{
    if (entries == nullptr)
    {
throw new ArgumentNullException("entries");
    }
Array<::RuntimeSceneCatalogEntry*> *copiedEntries = new Array<RuntimeSceneCatalogEntry*>(entries->get_Length());
Dictionary<std::string, ::RuntimeSceneCatalogEntry*> *entriesBySceneId = new Dictionary<std::string, ::RuntimeSceneCatalogEntry*>(StringComparer::get_OrdinalIgnoreCase());
for (int32_t index = 0; index < entries->get_Length(); index++) {
::RuntimeSceneCatalogEntry *entry = (*entries)[index];
    if (entry == nullptr)
    {
throw ([&]() {
auto __ctor_arg_00000140 = "Runtime scene catalog entries cannot contain null entries.";
auto __ctor_arg_00000141 = "entries";
return new ArgumentException(__ctor_arg_00000140, __ctor_arg_00000141);
})();
    }
    if (entriesBySceneId->ContainsKey(entry->SceneId))
    {
throw new InvalidOperationException(std::string("Runtime scene catalog contains duplicate scene id '") + entry->SceneId + std::string("'."));
    }
(*copiedEntries)[index] = entry;
entriesBySceneId->Add(entry->SceneId, entry);
}
this->Entries = copiedEntries;
this->EntriesBySceneId = entriesBySceneId;
}

bool RuntimeSceneCatalog::TryGetEntry__out1(std::string sceneId, ::RuntimeSceneCatalogEntry*& entry)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000142 = "Scene id is required.";
auto __ctor_arg_00000143 = "sceneId";
return new ArgumentException(__ctor_arg_00000142, __ctor_arg_00000143);
})();
    }
return this->EntriesBySceneId->TryGetValue(sceneId, entry);}

