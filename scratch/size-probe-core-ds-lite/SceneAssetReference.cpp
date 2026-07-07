#ifdef DrawText
#undef DrawText
#endif
#include "SceneAssetReference.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "runtime/native_string.hpp"
#include "SceneAssetReference.hpp"

::SceneAssetReferenceSourceKind SceneAssetReference::get_SourceKind()
{
return this->SourceKind;
}

const std::string& SceneAssetReference::get_RelativePath()
{
return this->RelativePath;
}

const std::string& SceneAssetReference::get_ProviderId()
{
return this->ProviderId;
}

const std::string& SceneAssetReference::get_AssetId()
{
return this->AssetId;
}

SceneAssetReference::SceneAssetReference(::SceneAssetReferenceSourceKind sourceKind, std::string relativePath, std::string providerId, std::string assetId) : SourceKind(), RelativePath(), ProviderId(), AssetId()
{
this->SourceKind = sourceKind;
this->RelativePath = relativePath;
this->ProviderId = providerId;
this->AssetId = assetId;
}

