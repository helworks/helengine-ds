#ifdef DrawText
#undef DrawText
#endif
#include "SceneAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneAssetReference.hpp"
#include "SceneCanvasProfile.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

SceneAsset::SceneAsset() : RootEntities(Array<SceneEntityAsset*>::Empty()), AssetReferences(Array<SceneAssetReference*>::Empty()), Physics3DSceneFeatureFlags(0), SceneSettingsValue(new ::SceneSettingsAsset())
{
}

Array<::SceneEntityAsset*>* SceneAsset::get_RootEntities()
{
return this->RootEntities;
}

void SceneAsset::set_RootEntities(Array<::SceneEntityAsset*>* value)
{
this->RootEntities = value;
}

Array<::SceneAssetReference*>* SceneAsset::get_AssetReferences()
{
return this->AssetReferences;
}

void SceneAsset::set_AssetReferences(Array<::SceneAssetReference*>* value)
{
this->AssetReferences = value;
}

::SceneSettingsAsset* SceneAsset::get_SceneSettings()
{
return this->SceneSettingsValue;}

void SceneAsset::set_SceneSettings(::SceneSettingsAsset* value)
{
::SceneSettingsAsset *newValue = (value != nullptr ? value : throw new ArgumentNullException("value"));
    if (this->SceneSettingsValue != nullptr && !(this->SceneSettingsValue == newValue))
    {
this->SceneSettingsValue->ReleaseOwnedValuesForNativeDelete();
delete this->SceneSettingsValue;
    }
this->SceneSettingsValue = newValue;
}

uint32_t SceneAsset::get_Physics3DSceneFeatureFlags()
{
return this->Physics3DSceneFeatureFlags;
}

void SceneAsset::set_Physics3DSceneFeatureFlags(uint32_t value)
{
this->Physics3DSceneFeatureFlags = value;
}

const std::string& SceneAsset::get_Id()
{
return Asset::get_Id();
}

void SceneAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t SceneAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void SceneAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

