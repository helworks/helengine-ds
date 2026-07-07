#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "RuntimeModel.hpp"
#include "ModelAsset.hpp"
#include "ContentManager.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "RuntimeMaterial.hpp"
#include "FontAsset.hpp"
#include "TextureAsset.hpp"
#include "RuntimeTexture.hpp"
#include "RenderManager2D.hpp"
#include "AnimationClipAsset.hpp"
#include "runtime/native_list.hpp"
#include "NativeOwnership.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "runtime/array.hpp"
#include "system/io/path.hpp"
#include "system/io/file.hpp"
#include "FontAssetBinarySerializer.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "float3.hpp"
#include "float2.hpp"
#include "ModelSubmeshAsset.hpp"
#include "SceneAssetReference.hpp"
#include "RuntimeSubmesh.hpp"
#include "ContentProcessorRegistration.hpp"
#include "runtime/native_type.hpp"
#include "IContentProcessor_1.hpp"
#include "CoreInitializationOptions.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "int2.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "runtime/native_event.hpp"
#include "RenderTarget.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "FontInfo.hpp"
#include "FontChar.hpp"
#include "FontTightMetrics.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryReader.hpp"
#include "system/string_comparer.hpp"
#include "system/io/file.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/io/path.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/io/stream.hpp"
#include "system/string_comparer.hpp"

const std::string& RuntimeSceneAssetReferenceResolver::get_LastTextLoadStage()
{
return this->LastTextLoadStage;
}

void RuntimeSceneAssetReferenceResolver::set_LastTextLoadStage(std::string value)
{
this->LastTextLoadStage = value;
}

const std::string& RuntimeSceneAssetReferenceResolver::get_LastTextFontRelativePath()
{
return this->LastTextFontRelativePath;
}

void RuntimeSceneAssetReferenceResolver::set_LastTextFontRelativePath(std::string value)
{
this->LastTextFontRelativePath = value;
}

const std::string& RuntimeSceneAssetReferenceResolver::get_LastTextureLoadStage()
{
return this->LastTextureLoadStage;
}

void RuntimeSceneAssetReferenceResolver::set_LastTextureLoadStage(std::string value)
{
this->LastTextureLoadStage = value;
}

const std::string& RuntimeSceneAssetReferenceResolver::get_LastTextureRelativePath()
{
return this->LastTextureRelativePath;
}

void RuntimeSceneAssetReferenceResolver::set_LastTextureRelativePath(std::string value)
{
this->LastTextureRelativePath = value;
}

const std::string& RuntimeSceneAssetReferenceResolver::get_LastFontDeserializeStage()
{
return FontAssetBinarySerializer::LastDeserializeStage;
}

void RuntimeSceneAssetReferenceResolver::BeginOwnedAssetTracking()
{
    if (this->ActiveOwnedTextures != nullptr || this->ActiveOwnedFonts != nullptr || this->ActiveOwnedModels != nullptr || this->ActiveOwnedMaterials != nullptr)
    {
throw new InvalidOperationException("Runtime scene asset tracking is already active.");
    }
this->ResetGeneratedRuntimeAssetCaches();
this->ActiveOwnedTextures = new List<::RuntimeTexture*>();
this->ActiveOwnedFonts = new List<::FontAsset*>();
this->ActiveResolvedFontsByPath = new Dictionary<std::string, ::FontAsset*>(StringComparer::get_OrdinalIgnoreCase());
this->ActiveOwnedModels = new List<::RuntimeModel*>();
this->ActiveOwnedMaterials = new List<::RuntimeMaterial*>();
}

void RuntimeSceneAssetReferenceResolver::CancelOwnedAssetTracking()
{
List<::RuntimeTexture*> *activeOwnedTextures = this->ActiveOwnedTextures;
List<::FontAsset*> *activeOwnedFonts = this->ActiveOwnedFonts;
Dictionary<std::string, ::FontAsset*> *activeResolvedFontsByPath = this->ActiveResolvedFontsByPath;
List<::RuntimeModel*> *activeOwnedModels = this->ActiveOwnedModels;
List<::RuntimeMaterial*> *activeOwnedMaterials = this->ActiveOwnedMaterials;
this->ActiveOwnedTextures = nullptr;
this->ActiveOwnedFonts = nullptr;
this->ActiveResolvedFontsByPath = nullptr;
this->ActiveOwnedModels = nullptr;
this->ActiveOwnedMaterials = nullptr;
this->ResetGeneratedRuntimeAssetCaches();
delete activeOwnedTextures;
delete activeOwnedFonts;
delete activeResolvedFontsByPath;
delete activeOwnedModels;
delete activeOwnedMaterials;
}

::RuntimeSceneOwnedAssetSet* RuntimeSceneAssetReferenceResolver::CompleteOwnedAssetTracking()
{
    if (this->ActiveOwnedTextures == nullptr || this->ActiveOwnedFonts == nullptr || this->ActiveOwnedModels == nullptr || this->ActiveOwnedMaterials == nullptr)
    {
throw new InvalidOperationException("Runtime scene asset tracking is not active.");
    }
List<::RuntimeTexture*> *ownedTextures = this->ActiveOwnedTextures;
List<::FontAsset*> *ownedFonts = this->ActiveOwnedFonts;
List<::RuntimeModel*> *ownedModels = this->ActiveOwnedModels;
List<::RuntimeMaterial*> *ownedMaterials = this->ActiveOwnedMaterials;
Dictionary<std::string, ::FontAsset*> *resolvedFontsByPath = this->ActiveResolvedFontsByPath;
this->ActiveOwnedTextures = nullptr;
this->ActiveOwnedFonts = nullptr;
this->ActiveResolvedFontsByPath = nullptr;
this->ActiveOwnedModels = nullptr;
this->ActiveOwnedMaterials = nullptr;
this->ResetGeneratedRuntimeAssetCaches();
delete resolvedFontsByPath;
return new ::RuntimeSceneOwnedAssetSet(ownedTextures, ownedFonts, ownedModels, ownedMaterials);}

RuntimeSceneAssetReferenceResolver::RuntimeSceneAssetReferenceResolver(::ContentManager* assetContentManager, std::string contentRootPath) : LastTextLoadStage(String::Empty), LastTextFontRelativePath(String::Empty), LastTextureLoadStage(String::Empty), LastTextureRelativePath(String::Empty), ContentRootPath(), AssetContentManager(), ActiveOwnedTextures(), ActiveOwnedFonts(), ActiveResolvedFontsByPath(), ActiveOwnedModels(), ActiveOwnedMaterials(), ActiveGeneratedModelsByKey(), ActiveGeneratedMaterialsByKey()
{
    if (assetContentManager == nullptr)
    {
throw new ArgumentNullException("assetContentManager");
    }
    if (String::IsNullOrWhiteSpace(contentRootPath))
    {
throw ([&]() {
auto __ctor_arg_00000137 = "Content root path must be provided.";
auto __ctor_arg_00000138 = "contentRootPath";
return new ArgumentException(__ctor_arg_00000137, __ctor_arg_00000138);
})();
    }
this->ContentRootPath = Path::GetFullPath(contentRootPath);
this->AssetContentManager = assetContentManager;
this->ActiveGeneratedModelsByKey = new Dictionary<std::string, ::RuntimeModel*>(StringComparer::get_Ordinal());
this->ActiveGeneratedMaterialsByKey = new Dictionary<std::string, ::RuntimeMaterial*>(StringComparer::get_Ordinal());
}

RuntimeSceneAssetReferenceResolver::RuntimeSceneAssetReferenceResolver(::ContentManager* assetContentManager, std::string contentRootPath, void* legacyShaderTarget) : RuntimeSceneAssetReferenceResolver(assetContentManager, contentRootPath)
{
    if (legacyShaderTarget == nullptr)
    {
throw new ArgumentNullException("legacyShaderTarget");
    }
}

::AnimationClipAsset* RuntimeSceneAssetReferenceResolver::ResolveAnimationClip(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
return this->AssetContentManager->Load<AnimationClipAsset*>(fullPath, RuntimeContentProcessorIds::AnimationClipAsset);}

::FontAsset* RuntimeSceneAssetReferenceResolver::ResolveFont(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
this->set_LastTextLoadStage("ResolveFontBegin");
this->set_LastTextFontRelativePath(reference->RelativePath);
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
    if (this->ActiveResolvedFontsByPath != nullptr)
    {
::FontAsset* cachedFontAsset;
    if (this->ActiveResolvedFontsByPath->TryGetValue(fullPath, cachedFontAsset))
    {
this->set_LastTextLoadStage("ResolveFontFromCache");
return cachedFontAsset;    }
    }
this->set_LastTextLoadStage("ResolveFontBeforeContentLoad");
::FontAsset *fontAsset = this->AssetContentManager->Load<FontAsset*>(fullPath, RuntimeContentProcessorIds::FontAsset);
this->AttachExternalCookedFontAtlasIfPresent(fontAsset);
this->set_LastTextLoadStage("ResolveFontAfterContentLoad");
    if (this->ActiveResolvedFontsByPath != nullptr)
    {
this->ActiveResolvedFontsByPath->Add(fullPath, fontAsset);
    }
this->TrackOwnedFont(fontAsset);
return fontAsset;}

::RuntimeMaterial* RuntimeSceneAssetReferenceResolver::ResolveMaterial(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
    if (reference->SourceKind == SceneAssetReferenceSourceKind::Generated)
    {
const std::string generatedAssetKey = this->BuildGeneratedAssetCacheKey(reference);
::RuntimeMaterial* generatedRuntimeMaterial;
    if (this->ActiveGeneratedMaterialsByKey->TryGetValue(generatedAssetKey, generatedRuntimeMaterial))
    {
this->TrackOwnedMaterial(generatedRuntimeMaterial);
return generatedRuntimeMaterial;    }
const std::string generatedFullPath = this->ResolveFileBackedAssetPath(reference);
::RuntimeMaterial *generatedRawRuntimeMaterial = Core::Instance->RenderManager3D->BuildMaterialFromRawAsset(this->AssetContentManager, this->ContentRootPath, generatedFullPath);
this->ActiveGeneratedMaterialsByKey->Add(generatedAssetKey, generatedRawRuntimeMaterial);
this->TrackOwnedMaterial(generatedRawRuntimeMaterial);
return generatedRawRuntimeMaterial;    }
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
::RuntimeMaterial *runtimeMaterial = Core::Instance->RenderManager3D->BuildMaterialFromRawAsset(this->AssetContentManager, this->ContentRootPath, fullPath);
this->TrackOwnedMaterial(runtimeMaterial);
return runtimeMaterial;}

::RuntimeModel* RuntimeSceneAssetReferenceResolver::ResolveModel(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
    if (reference->SourceKind == SceneAssetReferenceSourceKind::Generated)
    {
const std::string generatedAssetKey = this->BuildGeneratedAssetCacheKey(reference);
::RuntimeModel* generatedRuntimeModel;
    if (this->ActiveGeneratedModelsByKey->TryGetValue(generatedAssetKey, generatedRuntimeModel))
    {
this->TrackOwnedModel(generatedRuntimeModel);
return generatedRuntimeModel;    }
const std::string generatedFullPath = this->ResolveFileBackedAssetPath(reference);
::ModelAsset *generatedModelAsset = this->AssetContentManager->Load<ModelAsset*>(generatedFullPath, RuntimeContentProcessorIds::ModelAsset);
{
auto __finallyGuard_00000139 = he_cpp_make_scope_exit([&]() {
RuntimeSceneAssetReferenceResolver::ReleaseTransientModelAsset(generatedModelAsset);
});
::RuntimeModel *generatedModel = Core::Instance->RenderManager3D->BuildModelFromRaw(generatedModelAsset);
this->ActiveGeneratedModelsByKey->Add(generatedAssetKey, generatedModel);
this->TrackOwnedModel(generatedModel);
return generatedModel;}
    }
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
::ModelAsset *modelAsset = this->AssetContentManager->Load<ModelAsset*>(fullPath, RuntimeContentProcessorIds::ModelAsset);
{
auto __finallyGuard_0000013A = he_cpp_make_scope_exit([&]() {
RuntimeSceneAssetReferenceResolver::ReleaseTransientModelAsset(modelAsset);
});
::RuntimeModel *runtimeModel = Core::Instance->RenderManager3D->BuildModelFromRaw(modelAsset);
this->TrackOwnedModel(runtimeModel);
return runtimeModel;}
}

::RuntimeTexture* RuntimeSceneAssetReferenceResolver::ResolveTexture(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
this->set_LastTextureLoadStage("ResolveTextureBegin");
this->set_LastTextureRelativePath(reference->RelativePath);
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
this->set_LastTextureLoadStage("ResolveTextureBeforeContentLoad");
::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(fullPath, RuntimeContentProcessorIds::TextureAsset);
{
auto __finallyGuard_0000013B = he_cpp_make_scope_exit([&]() {
RuntimeSceneAssetReferenceResolver::ReleaseTransientTextureAsset(textureAsset);
});
this->set_LastTextureLoadStage("ResolveTextureBeforeBuild");
::RuntimeTexture *runtimeTexture = Core::Instance->RenderManager2D->BuildTextureFromRaw(textureAsset);
this->set_LastTextureLoadStage("ResolveTextureAfterBuild");
this->TrackOwnedTexture(runtimeTexture);
this->set_LastTextureLoadStage("ResolveTextureTracked");
return runtimeTexture;}
}

void RuntimeSceneAssetReferenceResolver::AttachExternalCookedFontAtlasIfPresent(::FontAsset* fontAsset)
{
    if (fontAsset == nullptr)
    {
throw new ArgumentNullException("fontAsset");
    }
    if (String::IsNullOrWhiteSpace(fontAsset->CookedAtlasTextureRelativePath))
    {
return;    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager2D == nullptr)
    {
throw new InvalidOperationException("External cooked font atlases require an initialized 2D render manager.");
    }
const std::string atlasFullPath = this->ResolvePackagedContentPath(fontAsset->CookedAtlasTextureRelativePath);
::TextureAsset *cookedAtlasTextureAsset = this->AssetContentManager->Load<TextureAsset*>(atlasFullPath, RuntimeContentProcessorIds::TextureAsset);
::RuntimeTexture *runtimeTexture = Core::Instance->RenderManager2D->BuildTextureFromRaw(cookedAtlasTextureAsset);
fontAsset->AttachProcessedTexture(runtimeTexture, cookedAtlasTextureAsset);
}

std::string RuntimeSceneAssetReferenceResolver::BuildGeneratedAssetCacheKey(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
    if (reference->SourceKind != SceneAssetReferenceSourceKind::Generated)
    {
throw new InvalidOperationException("Generated asset cache keys require generated scene asset references.");
    }
    if (String::IsNullOrWhiteSpace(reference->ProviderId))
    {
throw new InvalidOperationException("Generated scene asset references require a provider id.");
    }
    if (String::IsNullOrWhiteSpace(reference->AssetId))
    {
throw new InvalidOperationException("Generated scene asset references require an asset id.");
    }
return String::Concat(reference->ProviderId, "::", reference->AssetId);}

template <typename T>
void RuntimeSceneAssetReferenceResolver::DeleteTransientArray(Array<T>* values)
{
    if (values == nullptr || (values == Array<T>::Empty()))
    {
return;    }
delete values;
}

std::string RuntimeSceneAssetReferenceResolver::EnsureTrailingDirectorySeparator(std::string path)
{
    if (String::EndsWith(path, Path::DirectorySeparatorChar) || String::EndsWith(path, Path::AltDirectorySeparatorChar))
    {
return path;    }
return String::Concat(path, Path::DirectorySeparatorChar);}

void RuntimeSceneAssetReferenceResolver::ReleaseTransientModelAsset(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
Array<::float3> *positions = asset->Positions;
Array<::float3> *normals = asset->Normals;
Array<::float2> *texCoords = asset->TexCoords;
Array<uint16_t> *indices16 = asset->Indices16;
Array<uint32_t> *indices32 = asset->Indices32;
Array<::ModelSubmeshAsset*> *submeshes = asset->Submeshes;
asset->Positions = nullptr;
asset->Normals = nullptr;
asset->TexCoords = nullptr;
asset->Indices16 = nullptr;
asset->Indices32 = nullptr;
asset->Submeshes = nullptr;
    if (submeshes != nullptr)
    {
for (int32_t index = 0; index < submeshes->get_Length(); index++) {
delete (*submeshes)[index];
}
    }
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<float3>(positions);
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<float3>(normals);
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<float2>(texCoords);
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<uint16_t>(indices16);
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<uint32_t>(indices32);
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<ModelSubmeshAsset*>(submeshes);
delete asset;
}

void RuntimeSceneAssetReferenceResolver::ReleaseTransientTextureAsset(::TextureAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
Array<uint8_t> *colors = asset->Colors;
Array<uint8_t> *paletteColors = asset->PaletteColors;
asset->Colors = nullptr;
asset->PaletteColors = nullptr;
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<uint8_t>(colors);
RuntimeSceneAssetReferenceResolver::DeleteTransientArray<uint8_t>(paletteColors);
delete asset;
}

void RuntimeSceneAssetReferenceResolver::ResetGeneratedRuntimeAssetCaches()
{
this->ActiveGeneratedModelsByKey->Clear();
this->ActiveGeneratedMaterialsByKey->Clear();
}

std::string RuntimeSceneAssetReferenceResolver::ResolveFileBackedAssetPath(::SceneAssetReference* reference)
{
    if (reference->SourceKind != SceneAssetReferenceSourceKind::FileSystem && reference->SourceKind != SceneAssetReferenceSourceKind::Generated)
    {
throw new InvalidOperationException("Player builds currently require file-backed packaged scene references.");
    }
    if (String::IsNullOrWhiteSpace(reference->RelativePath))
    {
throw new InvalidOperationException("Packaged scene asset references must include a relative path.");
    }
const std::string fullPath = Path::GetFullPath(Path::Combine(this->ContentRootPath, reference->RelativePath));
const std::string contentRootPrefix = this->EnsureTrailingDirectorySeparator(this->ContentRootPath);
    if (!String::StartsWith(fullPath, contentRootPrefix, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException("Packaged scene asset reference path must stay inside the content root.");
    }
return fullPath;}

std::string RuntimeSceneAssetReferenceResolver::ResolveImportedTexturePackagePath(std::string assetId)
{
    if (String::IsNullOrWhiteSpace(assetId))
    {
throw new InvalidOperationException("Packaged material assets must include a diffuse texture asset id before resolving imported textures.");
    }
return Path::Combine(this->ContentRootPath, ImportedTextureDirectoryName, assetId);}

std::string RuntimeSceneAssetReferenceResolver::ResolvePackagedContentPath(std::string relativePath)
{
    if (String::IsNullOrWhiteSpace(relativePath))
    {
throw ([&]() {
auto __ctor_arg_0000013C = "Relative path must be provided.";
auto __ctor_arg_0000013D = "relativePath";
return new ArgumentException(__ctor_arg_0000013C, __ctor_arg_0000013D);
})();
    }
const std::string fullPath = Path::GetFullPath(Path::Combine(this->ContentRootPath, relativePath));
const std::string contentRootPrefix = this->EnsureTrailingDirectorySeparator(this->ContentRootPath);
    if (!String::StartsWith(fullPath, contentRootPrefix, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException("Packaged scene asset reference path must stay inside the content root.");
    }
return fullPath;}

void RuntimeSceneAssetReferenceResolver::TrackOwnedFont(::FontAsset* asset)
{
    if (asset == nullptr || this->ActiveOwnedFonts == nullptr)
    {
return;    }
    if (!this->ActiveOwnedFonts->Contains(asset))
    {
this->ActiveOwnedFonts->Add(asset);
    }
}

void RuntimeSceneAssetReferenceResolver::TrackOwnedMaterial(::RuntimeMaterial* asset)
{
    if (asset == nullptr || this->ActiveOwnedMaterials == nullptr)
    {
return;    }
    if (!this->ActiveOwnedMaterials->Contains(asset))
    {
this->ActiveOwnedMaterials->Add(asset);
    }
}

void RuntimeSceneAssetReferenceResolver::TrackOwnedModel(::RuntimeModel* asset)
{
    if (asset == nullptr || this->ActiveOwnedModels == nullptr)
    {
return;    }
    if (!this->ActiveOwnedModels->Contains(asset))
    {
this->ActiveOwnedModels->Add(asset);
    }
}

void RuntimeSceneAssetReferenceResolver::TrackOwnedTexture(::RuntimeTexture* asset)
{
    if (asset == nullptr || this->ActiveOwnedTextures == nullptr)
    {
return;    }
    if (!this->ActiveOwnedTextures->Contains(asset))
    {
this->ActiveOwnedTextures->Add(asset);
    }
}

bool RuntimeSceneAssetReferenceResolver::TryResolveSourceTexturePath__out2(std::string materialPath, std::string assetId, std::string& texturePath)
{
    if (String::IsNullOrWhiteSpace(materialPath))
    {
throw ([&]() {
auto __ctor_arg_0000013E = "Material path must be provided.";
auto __ctor_arg_0000013F = "materialPath";
return new ArgumentException(__ctor_arg_0000013E, __ctor_arg_0000013F);
})();
    }
    if (String::IsNullOrWhiteSpace(assetId))
    {
texturePath = String::Empty;
return false;    }
const std::string materialDirectoryPath = Path::GetDirectoryName(Path::GetFullPath(materialPath));
    if (String::IsNullOrWhiteSpace(materialDirectoryPath))
    {
texturePath = String::Empty;
return false;    }
const std::string candidateTexturePath = Path::IsPathRooted(assetId) ? Path::GetFullPath(assetId) : Path::GetFullPath(Path::Combine(materialDirectoryPath, assetId));
    if (File::Exists(candidateTexturePath))
    {
texturePath = candidateTexturePath;
return true;    }
texturePath = String::Empty;
return false;}

