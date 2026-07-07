#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ContentManager;
class RuntimeTexture;
class FontAsset;
class RuntimeModel;
class RuntimeMaterial;
class RuntimeSceneOwnedAssetSet;
class AnimationClipAsset;
class SceneAssetReference;
class ModelAsset;
class TextureAsset;

#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"

class RuntimeSceneAssetReferenceResolver
{
public:
    virtual ~RuntimeSceneAssetReferenceResolver() = default;

    std::string LastTextLoadStage;

    const std::string& get_LastTextLoadStage();
    void set_LastTextLoadStage(std::string value);

    std::string LastTextFontRelativePath;

    const std::string& get_LastTextFontRelativePath();
    void set_LastTextFontRelativePath(std::string value);

    std::string LastTextureLoadStage;

    const std::string& get_LastTextureLoadStage();
    void set_LastTextureLoadStage(std::string value);

    std::string LastTextureRelativePath;

    const std::string& get_LastTextureRelativePath();
    void set_LastTextureRelativePath(std::string value);

    const std::string& get_LastFontDeserializeStage();

    void BeginOwnedAssetTracking();

    void CancelOwnedAssetTracking();

    ::RuntimeSceneOwnedAssetSet* CompleteOwnedAssetTracking();

    RuntimeSceneAssetReferenceResolver(::ContentManager* assetContentManager, std::string contentRootPath);

    RuntimeSceneAssetReferenceResolver(::ContentManager* assetContentManager, std::string contentRootPath, void* legacyShaderTarget);

    ::AnimationClipAsset* ResolveAnimationClip(::SceneAssetReference* reference);

    ::FontAsset* ResolveFont(::SceneAssetReference* reference);

    ::RuntimeMaterial* ResolveMaterial(::SceneAssetReference* reference);

    ::RuntimeModel* ResolveModel(::SceneAssetReference* reference);

    ::RuntimeTexture* ResolveTexture(::SceneAssetReference* reference);
private:
    inline static const std::string ImportedTextureDirectoryName = "cooked/imported";

    inline static const std::string FontDirectoryName = "fonts";

    std::string ContentRootPath;

    ::ContentManager* AssetContentManager;

    List<::RuntimeTexture*>* ActiveOwnedTextures;

    List<::FontAsset*>* ActiveOwnedFonts;

    Dictionary<std::string, ::FontAsset*>* ActiveResolvedFontsByPath;

    List<::RuntimeModel*>* ActiveOwnedModels;

    List<::RuntimeMaterial*>* ActiveOwnedMaterials;

    Dictionary<std::string, ::RuntimeModel*>* ActiveGeneratedModelsByKey;

    Dictionary<std::string, ::RuntimeMaterial*>* ActiveGeneratedMaterialsByKey;

    void AttachExternalCookedFontAtlasIfPresent(::FontAsset* fontAsset);

    std::string BuildGeneratedAssetCacheKey(::SceneAssetReference* reference);

    template <typename T>
    static void DeleteTransientArray(Array<T>* values);

    std::string EnsureTrailingDirectorySeparator(std::string path);

    static void ReleaseTransientModelAsset(::ModelAsset* asset);

    static void ReleaseTransientTextureAsset(::TextureAsset* asset);

    void ResetGeneratedRuntimeAssetCaches();

    std::string ResolveFileBackedAssetPath(::SceneAssetReference* reference);

    std::string ResolveImportedTexturePackagePath(std::string assetId);

    std::string ResolvePackagedContentPath(std::string relativePath);

    void TrackOwnedFont(::FontAsset* asset);

    void TrackOwnedMaterial(::RuntimeMaterial* asset);

    void TrackOwnedModel(::RuntimeModel* asset);

    void TrackOwnedTexture(::RuntimeTexture* asset);

    bool TryResolveSourceTexturePath__out2(std::string materialPath, std::string assetId, std::string& texturePath);
};
