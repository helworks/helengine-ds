#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class SceneEntityAsset;
class SceneAssetReference;
class SceneSettingsAsset;

#include "Asset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class SceneAsset : public ::Asset
{
public:
    virtual ~SceneAsset() = default;

    SceneAsset();

    inline static const std::string FileExtension = ".helen";

    Array<::SceneEntityAsset*>* RootEntities;

    Array<::SceneEntityAsset*>* get_RootEntities();
    void set_RootEntities(Array<::SceneEntityAsset*>* value);

    Array<::SceneAssetReference*>* AssetReferences;

    Array<::SceneAssetReference*>* get_AssetReferences();
    void set_AssetReferences(Array<::SceneAssetReference*>* value);

    ::SceneSettingsAsset* get_SceneSettings();

    void set_SceneSettings(::SceneSettingsAsset* value);

    uint32_t Physics3DSceneFeatureFlags;

    uint32_t get_Physics3DSceneFeatureFlags();
    void set_Physics3DSceneFeatureFlags(uint32_t value);

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
private:
    ::SceneSettingsAsset* SceneSettingsValue;
};
