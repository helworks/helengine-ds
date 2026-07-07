#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class RuntimeSceneOwnedAssetSet;

#include "runtime/native_list.hpp"

class RuntimeSceneLoadResult
{
public:
    virtual ~RuntimeSceneLoadResult() = default;

    List<::Entity*>* RootEntities;

    List<::Entity*>* get_RootEntities();

    ::RuntimeSceneOwnedAssetSet* OwnedAssets;

    ::RuntimeSceneOwnedAssetSet* get_OwnedAssets();

    RuntimeSceneLoadResult(List<::Entity*>* rootEntities, ::RuntimeSceneOwnedAssetSet* ownedAssets);
};
