#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneAssetReference;
class EngineBinaryReader;

#include "runtime/native_string.hpp"
#include "SceneAssetReferenceSourceKind.hpp"

class SceneAssetReferenceFactory
{
public:
    virtual ~SceneAssetReferenceFactory() = default;

    static ::SceneAssetReference* CreateFileSystemAnimationClip(std::string relativePath);

    static ::SceneAssetReference* CreateFileSystemFont(std::string relativePath);

    static ::SceneAssetReference* CreateFileSystemMaterial(std::string relativePath);

    static ::SceneAssetReference* CreateFileSystemModel(std::string relativePath);

    static ::SceneAssetReference* CreateFileSystemTexture(std::string relativePath);

    static ::SceneAssetReference* ReadOptionalReference(::EngineBinaryReader* reader);

    static ::SceneAssetReference* ReadRequiredReference(::EngineBinaryReader* reader);

    static ::SceneAssetReference* Rehydrate(::SceneAssetReferenceSourceKind sourceKind, std::string relativePath, std::string providerId, std::string assetId);
private:
    static ::SceneAssetReference* CreateFileSystem(std::string relativePath);
};
