#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LoadedSceneRecord;
class RuntimeSceneCatalog;
class ContentManager;
class RuntimeSceneLoadService;
class ISceneIdPathResolver;
class ObjectManager;
class IRuntimeSceneTransitionDiagnosticsProvider;
class IRuntimeEntityDisposalDiagnosticsProvider;
class PendingSceneOperation;
class RuntimeTexture;
class FontAsset;
class RuntimeModel;
class RuntimeMaterial;
class Entity;
class RuntimeSceneOwnedAssetSet;
class SceneAsset;
class SceneComponentAssetRecord;
class SceneEntityAsset;
class SceneEntityPlatformAddedComponentAsset;
class SceneEntityPlatformComponentOverrideAsset;
class SceneEntityPlatformExistenceOverrideAsset;
class SceneEntityPlatformTransformOverrideAsset;
class SceneSettingsAsset;

#include "runtime/native_event.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_list.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/array.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class SceneManager
{
public:
    virtual ~SceneManager() = default;

    ::Event SceneLoading;

    ::Event SceneLoaded;

    ::Event SceneUnloading;

    ::Event SceneUnloaded;

    List<::LoadedSceneRecord*>* get_LoadedScenes();

    int32_t get_LoadedSceneRecordCapacity();

    int32_t get_PendingOperationCapacity();

    int32_t get_ActiveOwnedTextureReferenceCount();

    int32_t get_ActiveOwnedFontReferenceCount();

    int32_t get_ActiveOwnedModelReferenceCount();

    int32_t get_ActiveOwnedMaterialReferenceCount();

    std::string LastTraceStage;

    const std::string& get_LastTraceStage();
    void set_LastTraceStage(std::string value);

    std::string LastTraceSceneId;

    const std::string& get_LastTraceSceneId();
    void set_LastTraceSceneId(std::string value);

    int32_t LastTraceLoadedSceneCount;

    int32_t get_LastTraceLoadedSceneCount();
    void set_LastTraceLoadedSceneCount(int32_t value);

    int32_t LastTracePendingOperationCount;

    int32_t get_LastTracePendingOperationCount();
    void set_LastTracePendingOperationCount(int32_t value);

    int32_t LastTraceSerial;

    int32_t get_LastTraceSerial();
    void set_LastTraceSerial(int32_t value);

    void CommitPendingOperationsAtFrameBoundary();

    List<std::string>* GetLoadedSceneIds();

    bool IsSceneLoaded(std::string sceneId);

    void LoadScene(std::string sceneId, ::SceneLoadMode loadMode);

    void ReportEntityDisposalStage(std::string stage, ::Entity* entity, int32_t componentIndex);

    SceneManager(::RuntimeSceneCatalog* sceneCatalog, ::ContentManager* contentManager, ::RuntimeSceneLoadService* sceneLoadService, ::ObjectManager* objectManager, ::ISceneIdPathResolver* scenePathResolver, ::IRuntimeSceneTransitionDiagnosticsProvider* sceneTransitionDiagnosticsProvider, ::IRuntimeEntityDisposalDiagnosticsProvider* entityDisposalDiagnosticsProvider);

    bool TryGetLoadedScene__out1(std::string sceneId, ::LoadedSceneRecord*& loadedSceneRecord);

    void UnloadScene(std::string sceneId);
private:
    ::RuntimeSceneCatalog* SceneCatalog;

    ::ContentManager* ContentManager;

    ::RuntimeSceneLoadService* SceneLoadService;

    ::ISceneIdPathResolver* ScenePathResolver;

    ::ObjectManager* ObjectManager;

    ::IRuntimeSceneTransitionDiagnosticsProvider* SceneTransitionDiagnosticsProvider;

    ::IRuntimeEntityDisposalDiagnosticsProvider* EntityDisposalDiagnosticsProvider;

    List<::LoadedSceneRecord*>* LoadedSceneRecords;

    Dictionary<std::string, ::LoadedSceneRecord*>* LoadedSceneRecordsById;

    List<::PendingSceneOperation*>* PendingOperations;

    Dictionary<::RuntimeTexture*, int32_t>* ActiveOwnedTextureReferenceCounts;

    Dictionary<::FontAsset*, int32_t>* ActiveOwnedFontReferenceCounts;

    Dictionary<::RuntimeModel*, int32_t>* ActiveOwnedModelReferenceCounts;

    Dictionary<::RuntimeMaterial*, int32_t>* ActiveOwnedMaterialReferenceCounts;

    bool IsCommittingPendingOperations;

    template <typename T>
    static void DeleteTransientArray(Array<T>* values);

    void DisposeSceneRoots(List<::Entity*>* rootEntities);

    void DisposeUntrackedRootEntities();

    void FlushReleasedAssets();

    void LoadSceneImmediate(std::string sceneId, ::SceneLoadMode loadMode);

    void RecordTraceState(std::string stage, std::string sceneId);

    void RegisterOwnedAssets(::RuntimeSceneOwnedAssetSet* ownedAssets);

    void RegisterOwnedFonts(List<::FontAsset*>* ownedFonts);

    void RegisterOwnedMaterials(List<::RuntimeMaterial*>* ownedMaterials);

    void RegisterOwnedModels(List<::RuntimeModel*>* ownedModels);

    void RegisterOwnedTextures(List<::RuntimeTexture*>* ownedTextures);

    void ReleaseOwnedAsset(::RuntimeTexture* ownedAsset);

    void ReleaseOwnedAssets(::RuntimeSceneOwnedAssetSet* ownedAssets);

    void ReleaseOwnedFont(::FontAsset* ownedAsset);

    void ReleaseOwnedFonts(List<::FontAsset*>* ownedFonts);

    void ReleaseOwnedMaterial(::RuntimeMaterial* ownedAsset);

    void ReleaseOwnedMaterials(List<::RuntimeMaterial*>* ownedMaterials);

    void ReleaseOwnedModel(::RuntimeModel* ownedAsset);

    void ReleaseOwnedModels(List<::RuntimeModel*>* ownedModels);

    void ReleaseOwnedTextures(List<::RuntimeTexture*>* ownedTextures);

    static void ReleaseTransientSceneAsset(::SceneAsset* asset);

    static void ReleaseTransientSceneComponentAssetRecord(::SceneComponentAssetRecord* asset);

    static void ReleaseTransientSceneEntityAsset(::SceneEntityAsset* asset);

    static void ReleaseTransientSceneEntityPlatformAddedComponentAsset(::SceneEntityPlatformAddedComponentAsset* asset);

    static void ReleaseTransientSceneEntityPlatformComponentOverrideAsset(::SceneEntityPlatformComponentOverrideAsset* asset);

    static void ReleaseTransientSceneEntityPlatformExistenceOverrideAsset(::SceneEntityPlatformExistenceOverrideAsset* asset);

    static void ReleaseTransientSceneEntityPlatformTransformOverrideAsset(::SceneEntityPlatformTransformOverrideAsset* asset);

    static void ReleaseTransientSceneSettingsAsset(::SceneSettingsAsset* asset);

    void ResetPhysicsTimingForSingleLoad();

    std::string ResolveSceneContentPath(std::string sceneId);

    void UnloadAllScenes();

    void UnloadSceneImmediate(std::string sceneId);

    void UnloadScenesForSingleLoad();
};
