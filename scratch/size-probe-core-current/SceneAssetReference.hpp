#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "SceneAssetReferenceSourceKind.hpp"
#include "runtime/native_string.hpp"

class SceneAssetReference
{
public:
    virtual ~SceneAssetReference() = default;

    ::SceneAssetReferenceSourceKind SourceKind;

    ::SceneAssetReferenceSourceKind get_SourceKind();

    std::string RelativePath;

    const std::string& get_RelativePath();

    std::string ProviderId;

    const std::string& get_ProviderId();

    std::string AssetId;

    const std::string& get_AssetId();

    SceneAssetReference(::SceneAssetReferenceSourceKind sourceKind, std::string relativePath, std::string providerId, std::string assetId);
};
