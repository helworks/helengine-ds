#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeTexture;
class FontAsset;
class RuntimeModel;
class RuntimeMaterial;

#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class RuntimeSceneOwnedAssetSet
{
public:
    virtual ~RuntimeSceneOwnedAssetSet() = default;

    List<::RuntimeTexture*>* OwnedTextures;

    List<::RuntimeTexture*>* get_OwnedTextures();

    List<::FontAsset*>* OwnedFonts;

    List<::FontAsset*>* get_OwnedFonts();

    List<::RuntimeModel*>* OwnedModels;

    List<::RuntimeModel*>* get_OwnedModels();

    List<::RuntimeMaterial*>* OwnedMaterials;

    List<::RuntimeMaterial*>* get_OwnedMaterials();

    RuntimeSceneOwnedAssetSet(List<::RuntimeTexture*>* ownedTextures, List<::FontAsset*>* ownedFonts, List<::RuntimeModel*>* ownedModels, List<::RuntimeMaterial*>* ownedMaterials);
};
