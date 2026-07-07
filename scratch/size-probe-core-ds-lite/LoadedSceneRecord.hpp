#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class RuntimeSceneOwnedAssetSet;

#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"

class LoadedSceneRecord
{
public:
    virtual ~LoadedSceneRecord() = default;

    std::string SceneId;

    const std::string& get_SceneId();

    std::string CookedRelativePath;

    const std::string& get_CookedRelativePath();

    List<::Entity*>* RootEntities;

    List<::Entity*>* get_RootEntities();

    ::RuntimeSceneOwnedAssetSet* OwnedAssets;

    ::RuntimeSceneOwnedAssetSet* get_OwnedAssets();

    bool DontUnload;

    bool get_DontUnload();

    LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, ::RuntimeSceneOwnedAssetSet* ownedAssets, bool dontUnload);
};
