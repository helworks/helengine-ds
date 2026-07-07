#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "runtime/native_list.hpp"
#include "RuntimeTexture.hpp"
#include "FontAsset.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

List<::RuntimeTexture*>* RuntimeSceneOwnedAssetSet::get_OwnedTextures()
{
return this->OwnedTextures;
}

List<::FontAsset*>* RuntimeSceneOwnedAssetSet::get_OwnedFonts()
{
return this->OwnedFonts;
}

List<::RuntimeModel*>* RuntimeSceneOwnedAssetSet::get_OwnedModels()
{
return this->OwnedModels;
}

List<::RuntimeMaterial*>* RuntimeSceneOwnedAssetSet::get_OwnedMaterials()
{
return this->OwnedMaterials;
}

RuntimeSceneOwnedAssetSet::RuntimeSceneOwnedAssetSet(List<::RuntimeTexture*>* ownedTextures, List<::FontAsset*>* ownedFonts, List<::RuntimeModel*>* ownedModels, List<::RuntimeMaterial*>* ownedMaterials) : OwnedTextures(), OwnedFonts(), OwnedModels(), OwnedMaterials()
{
this->OwnedTextures = (ownedTextures != nullptr ? ownedTextures : throw new ArgumentNullException("ownedTextures"));
this->OwnedFonts = (ownedFonts != nullptr ? ownedFonts : throw new ArgumentNullException("ownedFonts"));
this->OwnedModels = (ownedModels != nullptr ? ownedModels : throw new ArgumentNullException("ownedModels"));
this->OwnedMaterials = (ownedMaterials != nullptr ? ownedMaterials : throw new ArgumentNullException("ownedMaterials"));
}

