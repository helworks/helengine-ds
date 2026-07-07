#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeSceneCatalogEntry;

#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"

class RuntimeSceneCatalog
{
public:
    virtual ~RuntimeSceneCatalog() = default;

    Array<::RuntimeSceneCatalogEntry*>* Entries;

    Array<::RuntimeSceneCatalogEntry*>* get_Entries();

    RuntimeSceneCatalog(Array<::RuntimeSceneCatalogEntry*>* entries);

    bool TryGetEntry__out1(std::string sceneId, ::RuntimeSceneCatalogEntry*& entry);
private:
    Dictionary<std::string, ::RuntimeSceneCatalogEntry*>* EntriesBySceneId;
};
