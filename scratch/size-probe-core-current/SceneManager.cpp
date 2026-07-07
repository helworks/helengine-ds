#ifdef DrawText
#undef DrawText
#endif
#include "SceneManager.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_dictionary.hpp"
#include "SceneLoadingEventArgs.hpp"
#include "SceneAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeSceneLoadResult.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "LoadedSceneRecord.hpp"
#include "SceneLoadedEventArgs.hpp"
#include "SceneUnloadingEventArgs.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "SceneUnloadedEventArgs.hpp"
#include "NativeOwnership.hpp"
#include "Core.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "ISceneIdPathResolver.hpp"
#include "runtime/array.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "FontAsset.hpp"
#include "RenderManager2D.hpp"
#include "RuntimeTexture.hpp"
#include "RenderManager3D.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "PendingSceneOperation.hpp"
#include "PendingSceneOperationKind.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "Entity.hpp"
#include "SceneLoadMode.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "SceneManager.hpp"
#include "SceneAssetReference.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "runtime/native_event.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "ObjectManager.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "ContentProcessorRegistration.hpp"
#include "runtime/native_type.hpp"
#include "IContentProcessor_1.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "Component.hpp"
#include "CoreInitializationOptions.hpp"
#include "IEntityFactory.hpp"
#include "int2.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "SceneCanvasProfile.hpp"
#include "FontInfo.hpp"
#include "FontChar.hpp"
#include "TextureAsset.hpp"
#include "float2.hpp"
#include "FontTightMetrics.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "ModelAsset.hpp"
#include "RenderTarget.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "RuntimeSubmesh.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "float4x4.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "system/string_comparer.hpp"
#include "system/action.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/string_comparer.hpp"

List<::LoadedSceneRecord*>* SceneManager::get_LoadedScenes()
{
return this->LoadedSceneRecords;
}

int32_t SceneManager::get_LoadedSceneRecordCapacity()
{
return this->LoadedSceneRecords->get_Capacity();
}

int32_t SceneManager::get_PendingOperationCapacity()
{
return this->PendingOperations->get_Capacity();
}

int32_t SceneManager::get_ActiveOwnedTextureReferenceCount()
{
return this->ActiveOwnedTextureReferenceCounts->get_Count();
}

int32_t SceneManager::get_ActiveOwnedFontReferenceCount()
{
return this->ActiveOwnedFontReferenceCounts->get_Count();
}

int32_t SceneManager::get_ActiveOwnedModelReferenceCount()
{
return this->ActiveOwnedModelReferenceCounts->get_Count();
}

int32_t SceneManager::get_ActiveOwnedMaterialReferenceCount()
{
return this->ActiveOwnedMaterialReferenceCounts->get_Count();
}

const std::string& SceneManager::get_LastTraceStage()
{
return this->LastTraceStage;
}

void SceneManager::set_LastTraceStage(std::string value)
{
this->LastTraceStage = value;
}

const std::string& SceneManager::get_LastTraceSceneId()
{
return this->LastTraceSceneId;
}

void SceneManager::set_LastTraceSceneId(std::string value)
{
this->LastTraceSceneId = value;
}

int32_t SceneManager::get_LastTraceLoadedSceneCount()
{
return this->LastTraceLoadedSceneCount;
}

void SceneManager::set_LastTraceLoadedSceneCount(int32_t value)
{
this->LastTraceLoadedSceneCount = value;
}

int32_t SceneManager::get_LastTracePendingOperationCount()
{
return this->LastTracePendingOperationCount;
}

void SceneManager::set_LastTracePendingOperationCount(int32_t value)
{
this->LastTracePendingOperationCount = value;
}

int32_t SceneManager::get_LastTraceSerial()
{
return this->LastTraceSerial;
}

void SceneManager::set_LastTraceSerial(int32_t value)
{
this->LastTraceSerial = value;
}

void SceneManager::CommitPendingOperationsAtFrameBoundary()
{
    if (this->PendingOperations->get_Count() == 0)
    {
return;    }
this->RecordTraceState("CommitPendingOperationsAtFrameBoundaryBegin", String::Empty);
const int32_t operationCountToCommit = this->PendingOperations->get_Count();
bool shouldFlushReleasedAssetsAtFrameBoundary = false;
this->IsCommittingPendingOperations = true;
{
auto __finallyGuard_0000015C = he_cpp_make_scope_exit([&]() {
this->IsCommittingPendingOperations = false;
});
for (int32_t operationIndex = 0; operationIndex < operationCountToCommit; operationIndex++) {
::PendingSceneOperation *operation = (*this->PendingOperations).get_Item(static_cast<int32_t>(0));
this->PendingOperations->RemoveAt(static_cast<int32_t>(0));
this->RecordTraceState("CommitPendingOperationsAtFrameBoundaryOperation", operation->SceneId);
    if (operation->OperationKind == PendingSceneOperationKind::Load && shouldFlushReleasedAssetsAtFrameBoundary)
    {
this->FlushReleasedAssets();
shouldFlushReleasedAssetsAtFrameBoundary = false;
    }
    if (operation->OperationKind == PendingSceneOperationKind::Load)
    {
this->LoadSceneImmediate(operation->SceneId, static_cast<SceneLoadMode>(operation->LoadMode));
    }
else {
this->UnloadSceneImmediate(operation->SceneId);
shouldFlushReleasedAssetsAtFrameBoundary = true;
}
delete operation;
}
}
    if (shouldFlushReleasedAssetsAtFrameBoundary)
    {
this->FlushReleasedAssets();
    }
this->RecordTraceState("CommitPendingOperationsAtFrameBoundaryEnd", String::Empty);
}

List<std::string>* SceneManager::GetLoadedSceneIds()
{
List<std::string> *sceneIds = new List<std::string>(static_cast<int32_t>(this->LoadedSceneRecords->get_Count()));
for (int32_t index = 0; index < this->LoadedSceneRecords->get_Count(); index++) {
sceneIds->Add((*this->LoadedSceneRecords).get_Item(static_cast<int32_t>(index))->SceneId);
}
return sceneIds;}

bool SceneManager::IsSceneLoaded(std::string sceneId)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_0000015D = "Scene id is required.";
auto __ctor_arg_0000015E = "sceneId";
return new ArgumentException(__ctor_arg_0000015D, __ctor_arg_0000015E);
})();
    }
return this->LoadedSceneRecordsById->ContainsKey(sceneId);}

void SceneManager::LoadScene(std::string sceneId, ::SceneLoadMode loadMode)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_0000015F = "Scene id is required.";
auto __ctor_arg_00000160 = "sceneId";
return new ArgumentException(__ctor_arg_0000015F, __ctor_arg_00000160);
})();
    }
this->RecordTraceState("LoadSceneRequest", sceneId);
this->PendingOperations->Add(PendingSceneOperation::CreateLoad(sceneId, static_cast<SceneLoadMode>(loadMode)));
this->RecordTraceState("LoadSceneDeferred", sceneId);
}

void SceneManager::ReportEntityDisposalStage(std::string stage, ::Entity* entity, int32_t componentIndex)
{
    if (this->EntityDisposalDiagnosticsProvider == nullptr)
    {
return;    }
const int32_t childCount = entity != nullptr && entity->get_Children() != nullptr ? entity->get_Children()->get_Count() : 0;
const int32_t componentCount = entity != nullptr && entity->get_Components() != nullptr ? entity->get_Components()->get_Count() : 0;
this->EntityDisposalDiagnosticsProvider->ReportEntityDisposalStage(stage, static_cast<int32_t>(childCount), static_cast<int32_t>(componentCount), static_cast<int32_t>(componentIndex));
}

SceneManager::SceneManager(::RuntimeSceneCatalog* sceneCatalog, ::ContentManager* contentManager, ::RuntimeSceneLoadService* sceneLoadService, ::ObjectManager* objectManager, ::ISceneIdPathResolver* scenePathResolver, ::IRuntimeSceneTransitionDiagnosticsProvider* sceneTransitionDiagnosticsProvider, ::IRuntimeEntityDisposalDiagnosticsProvider* entityDisposalDiagnosticsProvider) : SceneLoading(), SceneLoaded(), SceneUnloading(), SceneUnloaded(), LastTraceStage(String::Empty), LastTraceSceneId(String::Empty), LastTraceLoadedSceneCount(0), LastTracePendingOperationCount(0), LastTraceSerial(0), SceneCatalog(), ContentManager(), SceneLoadService(), ScenePathResolver(), ObjectManager(), SceneTransitionDiagnosticsProvider(), EntityDisposalDiagnosticsProvider(), LoadedSceneRecords(), LoadedSceneRecordsById(), PendingOperations(), ActiveOwnedTextureReferenceCounts(), ActiveOwnedFontReferenceCounts(), ActiveOwnedModelReferenceCounts(), ActiveOwnedMaterialReferenceCounts(), IsCommittingPendingOperations()
{
    if (sceneCatalog == nullptr && scenePathResolver == nullptr)
    {
throw new InvalidOperationException("A runtime scene manager requires either a scene catalog or a scene path resolver.");
    }
this->SceneCatalog = sceneCatalog;
this->ContentManager = (contentManager != nullptr ? contentManager : throw new ArgumentNullException("contentManager"));
this->SceneLoadService = (sceneLoadService != nullptr ? sceneLoadService : throw new ArgumentNullException("sceneLoadService"));
this->ObjectManager = (objectManager != nullptr ? objectManager : throw new ArgumentNullException("objectManager"));
this->ScenePathResolver = scenePathResolver;
this->SceneTransitionDiagnosticsProvider = sceneTransitionDiagnosticsProvider;
this->EntityDisposalDiagnosticsProvider = entityDisposalDiagnosticsProvider;
this->LoadedSceneRecords = new List<::LoadedSceneRecord*>();
this->LoadedSceneRecordsById = new Dictionary<std::string, ::LoadedSceneRecord*>(StringComparer::get_OrdinalIgnoreCase());
this->PendingOperations = new List<::PendingSceneOperation*>();
this->ActiveOwnedTextureReferenceCounts = new Dictionary<::RuntimeTexture*, int32_t>();
this->ActiveOwnedFontReferenceCounts = new Dictionary<::FontAsset*, int32_t>();
this->ActiveOwnedModelReferenceCounts = new Dictionary<::RuntimeModel*, int32_t>();
this->ActiveOwnedMaterialReferenceCounts = new Dictionary<::RuntimeMaterial*, int32_t>();
}

bool SceneManager::TryGetLoadedScene__out1(std::string sceneId, ::LoadedSceneRecord*& loadedSceneRecord)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000161 = "Scene id is required.";
auto __ctor_arg_00000162 = "sceneId";
return new ArgumentException(__ctor_arg_00000161, __ctor_arg_00000162);
})();
    }
return this->LoadedSceneRecordsById->TryGetValue(sceneId, loadedSceneRecord);}

void SceneManager::UnloadScene(std::string sceneId)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000163 = "Scene id is required.";
auto __ctor_arg_00000164 = "sceneId";
return new ArgumentException(__ctor_arg_00000163, __ctor_arg_00000164);
})();
    }
this->PendingOperations->Add(PendingSceneOperation::CreateUnload(sceneId));
this->RecordTraceState("UnloadSceneDeferred", sceneId);
}

template <typename T>
void SceneManager::DeleteTransientArray(Array<T>* values)
{
    if (values == nullptr || (values == Array<T>::Empty()))
    {
return;    }
delete values;
}

void SceneManager::DisposeSceneRoots(List<::Entity*>* rootEntities)
{
    if (rootEntities == nullptr)
    {
throw new ArgumentNullException("rootEntities");
    }
for (int32_t index = rootEntities->get_Count() - 1; index >= 0; index--) {
::Entity *rootEntity = (*rootEntities).get_Item(static_cast<int32_t>(index));
this->ReportEntityDisposalStage("BeforeRootDispose", rootEntity, static_cast<int32_t>(-1));
if (rootEntity != nullptr)
{
rootEntity->Dispose();
delete rootEntity;
}
this->ReportEntityDisposalStage("AfterRootDispose", nullptr, static_cast<int32_t>(-1));
}
}

void SceneManager::DisposeUntrackedRootEntities()
{
List<::Entity*> *rootEntities = new List<::Entity*>();
{
auto __finallyGuard_00000165 = he_cpp_make_scope_exit([&]() {
delete rootEntities;
});
for (int32_t index = 0; index < this->ObjectManager->Entities->get_Count(); index++) {
::Entity *entity = (*this->ObjectManager->Entities).get_Item(static_cast<int32_t>(index));
    if (entity->Parent == nullptr)
    {
rootEntities->Add(entity);
    }
}
for (int32_t index = rootEntities->get_Count() - 1; index >= 0; index--) {
::Entity *rootEntity = (*rootEntities).get_Item(static_cast<int32_t>(index));
this->ReportEntityDisposalStage("BeforeUntrackedRootDispose", rootEntity, static_cast<int32_t>(-1));
if (rootEntity != nullptr)
{
rootEntity->Dispose();
delete rootEntity;
}
this->ReportEntityDisposalStage("AfterUntrackedRootDispose", nullptr, static_cast<int32_t>(-1));
}
}
}

void SceneManager::FlushReleasedAssets()
{
    if (Core::Instance == nullptr || Core::Instance->RenderManager2D == nullptr)
    {
throw new InvalidOperationException("Deferred runtime texture release flushing requires an initialized 2D render manager.");
    }
Core::Instance->RenderManager2D->FlushReleasedTextures();
    if (Core::Instance->RenderManager3D == nullptr)
    {
throw new InvalidOperationException("Deferred runtime asset release flushing requires an initialized 3D render manager.");
    }
Core::Instance->RenderManager3D->FlushReleasedAssets();
Core::Instance->RenderManager2D->FlushReleasedTextures();
}

void SceneManager::LoadSceneImmediate(std::string sceneId, ::SceneLoadMode loadMode)
{
this->RecordTraceState("LoadSceneImmediateBegin", sceneId);
const std::string sceneContentPath = this->ResolveSceneContentPath(sceneId);
    if (this->LoadedSceneRecordsById->ContainsKey(sceneId))
    {
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' is already loaded."));
    }
    if (loadMode == SceneLoadMode::Single)
    {
    if (this->LoadedSceneRecords->get_Count() == 0)
    {
this->RecordTraceState("LoadSceneImmediateDisposeUntrackedRoots", sceneId);
this->DisposeUntrackedRootEntities();
    }
else {
this->RecordTraceState("LoadSceneImmediateUnloadSingleModeScenes", sceneId);
this->UnloadScenesForSingleLoad();
}
this->RecordTraceState("LoadSceneImmediateFlushReleasedTextures", sceneId);
this->FlushReleasedAssets();
this->RecordTraceState("LoadSceneImmediateResetPhysicsTiming", sceneId);
this->ResetPhysicsTimingForSingleLoad();
    }
::SceneLoadingEventArgs *sceneLoadingEventArgs = new ::SceneLoadingEventArgs(sceneId, sceneContentPath);
{
auto __finallyGuard_00000166 = he_cpp_make_scope_exit([&]() {
delete sceneLoadingEventArgs;
});
this->SceneLoading.Invoke(this, sceneLoadingEventArgs);
}
this->RecordTraceState("LoadSceneImmediateBeforeContentLoad", sceneId);
::SceneAsset *sceneAsset = this->ContentManager->Load<SceneAsset*>(sceneContentPath, RuntimeContentProcessorIds::SceneAsset);
{
auto __finallyGuard_00000167 = he_cpp_make_scope_exit([&]() {
SceneManager::ReleaseTransientSceneAsset(sceneAsset);
});
const bool dontUnload = sceneAsset->get_SceneSettings() != nullptr && sceneAsset->get_SceneSettings()->DontUnload;
this->RecordTraceState("LoadSceneImmediateBeforeSceneLoadServiceLoad", sceneId);
::RuntimeSceneLoadResult *loadResult = this->SceneLoadService->LoadTracked(sceneAsset);
{
auto __finallyGuard_00000168 = he_cpp_make_scope_exit([&]() {
delete loadResult;
});
this->RecordTraceState("LoadSceneImmediateAfterSceneLoadServiceLoad", sceneId);
::LoadedSceneRecord *loadedSceneRecord = new ::LoadedSceneRecord(sceneId, sceneContentPath, loadResult->RootEntities, loadResult->OwnedAssets, dontUnload);
this->RecordTraceState("LoadSceneImmediateBeforeLoadedSceneRecordTrack", sceneId);
this->LoadedSceneRecords->Add(loadedSceneRecord);
this->LoadedSceneRecordsById->Add(loadedSceneRecord->SceneId, loadedSceneRecord);
this->RegisterOwnedAssets(loadedSceneRecord->OwnedAssets);
::SceneLoadedEventArgs *sceneLoadedEventArgs = new ::SceneLoadedEventArgs(loadedSceneRecord->SceneId, loadedSceneRecord->CookedRelativePath, loadedSceneRecord->RootEntities);
{
auto __finallyGuard_00000169 = he_cpp_make_scope_exit([&]() {
delete sceneLoadedEventArgs;
});
this->SceneLoaded.Invoke(this, sceneLoadedEventArgs);
}
this->RecordTraceState("LoadSceneImmediateEnd", sceneId);
}
}
}

void SceneManager::RecordTraceState(std::string stage, std::string sceneId)
{
this->LastTraceSerial++;
this->set_LastTraceStage(stage);
this->set_LastTraceSceneId(sceneId);
this->set_LastTraceLoadedSceneCount(this->LoadedSceneRecords->get_Count());
this->set_LastTracePendingOperationCount(this->PendingOperations->get_Count());
if (Core::Instance != nullptr)
{
Core::Instance->ReportSceneTransitionStage(std::string("SceneManager:") + this->LastTraceStage);
}
if (this->SceneTransitionDiagnosticsProvider != nullptr)
{
this->SceneTransitionDiagnosticsProvider->ReportSceneTransitionStage(stage, this->LastTraceSceneId, this->LastTraceLoadedSceneCount, this->LastTracePendingOperationCount);
}
}

void SceneManager::RegisterOwnedAssets(::RuntimeSceneOwnedAssetSet* ownedAssets)
{
    if (ownedAssets == nullptr)
    {
throw new ArgumentNullException("ownedAssets");
    }
this->RegisterOwnedTextures(ownedAssets->OwnedTextures);
this->RegisterOwnedFonts(ownedAssets->OwnedFonts);
this->RegisterOwnedModels(ownedAssets->OwnedModels);
this->RegisterOwnedMaterials(ownedAssets->OwnedMaterials);
}

void SceneManager::RegisterOwnedFonts(List<::FontAsset*>* ownedFonts)
{
    if (ownedFonts == nullptr)
    {
throw new ArgumentNullException("ownedFonts");
    }
for (int32_t assetIndex = 0; assetIndex < ownedFonts->get_Count(); assetIndex++) {
::FontAsset *ownedAsset = (*ownedFonts).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (this->ActiveOwnedFontReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
(*this->ActiveOwnedFontReferenceCounts).get_Item(ownedAsset) = existingReferenceCount + 1;
    }
else {
this->ActiveOwnedFontReferenceCounts->Add(ownedAsset, static_cast<int32_t>(1));
}
}
}

void SceneManager::RegisterOwnedMaterials(List<::RuntimeMaterial*>* ownedMaterials)
{
    if (ownedMaterials == nullptr)
    {
throw new ArgumentNullException("ownedMaterials");
    }
for (int32_t assetIndex = 0; assetIndex < ownedMaterials->get_Count(); assetIndex++) {
::RuntimeMaterial *ownedAsset = (*ownedMaterials).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (this->ActiveOwnedMaterialReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
(*this->ActiveOwnedMaterialReferenceCounts).get_Item(ownedAsset) = existingReferenceCount + 1;
    }
else {
this->ActiveOwnedMaterialReferenceCounts->Add(ownedAsset, static_cast<int32_t>(1));
}
}
}

void SceneManager::RegisterOwnedModels(List<::RuntimeModel*>* ownedModels)
{
    if (ownedModels == nullptr)
    {
throw new ArgumentNullException("ownedModels");
    }
for (int32_t assetIndex = 0; assetIndex < ownedModels->get_Count(); assetIndex++) {
::RuntimeModel *ownedAsset = (*ownedModels).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (this->ActiveOwnedModelReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
(*this->ActiveOwnedModelReferenceCounts).get_Item(ownedAsset) = existingReferenceCount + 1;
    }
else {
this->ActiveOwnedModelReferenceCounts->Add(ownedAsset, static_cast<int32_t>(1));
}
}
}

void SceneManager::RegisterOwnedTextures(List<::RuntimeTexture*>* ownedTextures)
{
    if (ownedTextures == nullptr)
    {
throw new ArgumentNullException("ownedTextures");
    }
for (int32_t assetIndex = 0; assetIndex < ownedTextures->get_Count(); assetIndex++) {
::RuntimeTexture *ownedAsset = (*ownedTextures).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (this->ActiveOwnedTextureReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
(*this->ActiveOwnedTextureReferenceCounts).get_Item(ownedAsset) = existingReferenceCount + 1;
    }
else {
this->ActiveOwnedTextureReferenceCounts->Add(ownedAsset, static_cast<int32_t>(1));
}
}
}

void SceneManager::ReleaseOwnedAsset(::RuntimeTexture* ownedAsset)
{
    if (ownedAsset == nullptr)
    {
throw new ArgumentNullException("ownedAsset");
    }
    if (ownedAsset->IsDisposed)
    {
return;    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager2D == nullptr)
    {
throw new InvalidOperationException("Runtime texture release requires an initialized 2D render manager.");
    }
Core::Instance->RenderManager2D->ReleaseTexture(ownedAsset);
}

void SceneManager::ReleaseOwnedAssets(::RuntimeSceneOwnedAssetSet* ownedAssets)
{
    if (ownedAssets == nullptr)
    {
throw new ArgumentNullException("ownedAssets");
    }
this->RecordTraceState("ReleaseOwnedAssetsBeforeFonts", this->LastTraceSceneId);
this->ReleaseOwnedFonts(ownedAssets->OwnedFonts);
this->RecordTraceState("ReleaseOwnedAssetsAfterFonts", this->LastTraceSceneId);
this->RecordTraceState("ReleaseOwnedAssetsBeforeTextures", this->LastTraceSceneId);
this->ReleaseOwnedTextures(ownedAssets->OwnedTextures);
this->RecordTraceState("ReleaseOwnedAssetsAfterTextures", this->LastTraceSceneId);
this->RecordTraceState("ReleaseOwnedAssetsBeforeModels", this->LastTraceSceneId);
this->ReleaseOwnedModels(ownedAssets->OwnedModels);
this->RecordTraceState("ReleaseOwnedAssetsAfterModels", this->LastTraceSceneId);
this->RecordTraceState("ReleaseOwnedAssetsBeforeMaterials", this->LastTraceSceneId);
this->ReleaseOwnedMaterials(ownedAssets->OwnedMaterials);
this->RecordTraceState("ReleaseOwnedAssetsAfterMaterials", this->LastTraceSceneId);
}

void SceneManager::ReleaseOwnedFont(::FontAsset* ownedAsset)
{
    if (ownedAsset == nullptr)
    {
throw new ArgumentNullException("ownedAsset");
    }
    if (ownedAsset->IsDisposed)
    {
return;    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager2D == nullptr)
    {
throw new InvalidOperationException("Font asset release requires an initialized 2D render manager.");
    }
Core::Instance->RenderManager2D->ReleaseFont(ownedAsset);
}

void SceneManager::ReleaseOwnedFonts(List<::FontAsset*>* ownedFonts)
{
    if (ownedFonts == nullptr)
    {
throw new ArgumentNullException("ownedFonts");
    }
for (int32_t assetIndex = 0; assetIndex < ownedFonts->get_Count(); assetIndex++) {
::FontAsset *ownedAsset = (*ownedFonts).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (!this->ActiveOwnedFontReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
throw new InvalidOperationException("Scene-owned font asset was not tracked before release.");
    }
    if (existingReferenceCount > 1)
    {
(*this->ActiveOwnedFontReferenceCounts).get_Item(ownedAsset) = existingReferenceCount - 1;
continue;
    }
this->ActiveOwnedFontReferenceCounts->Remove(ownedAsset);
this->ReleaseOwnedFont(ownedAsset);
}
}

void SceneManager::ReleaseOwnedMaterial(::RuntimeMaterial* ownedAsset)
{
    if (ownedAsset == nullptr)
    {
throw new ArgumentNullException("ownedAsset");
    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr)
    {
throw new InvalidOperationException("Runtime material release requires an initialized 3D render manager.");
    }
this->RecordTraceState("MatBeforeRM3D", this->LastTraceSceneId);
Core::Instance->RenderManager3D->ReleaseMaterial(ownedAsset);
this->RecordTraceState("MatAfterRM3D", this->LastTraceSceneId);
}

void SceneManager::ReleaseOwnedMaterials(List<::RuntimeMaterial*>* ownedMaterials)
{
this->RecordTraceState("MatBegin", this->LastTraceSceneId);
    if (ownedMaterials == nullptr)
    {
throw new ArgumentNullException("ownedMaterials");
    }
for (int32_t assetIndex = 0; assetIndex < ownedMaterials->get_Count(); assetIndex++) {
this->RecordTraceState("MatLoop", this->LastTraceSceneId);
::RuntimeMaterial *ownedAsset = (*ownedMaterials).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
this->RecordTraceState("MatNullSkip", this->LastTraceSceneId);
continue;
    }
this->RecordTraceState("MatBeforeRef", this->LastTraceSceneId);
int32_t existingReferenceCount;
    if (!this->ActiveOwnedMaterialReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
throw new InvalidOperationException("Scene-owned runtime material was not tracked before release.");
    }
this->RecordTraceState("MatAfterRef", this->LastTraceSceneId);
    if (existingReferenceCount > 1)
    {
this->RecordTraceState("MatBeforeDec", this->LastTraceSceneId);
(*this->ActiveOwnedMaterialReferenceCounts).get_Item(ownedAsset) = existingReferenceCount - 1;
this->RecordTraceState("MatAfterDec", this->LastTraceSceneId);
continue;
    }
this->RecordTraceState("MatBeforeRemove", this->LastTraceSceneId);
this->ActiveOwnedMaterialReferenceCounts->Remove(ownedAsset);
this->RecordTraceState("MatAfterRemove", this->LastTraceSceneId);
this->RecordTraceState("MatBeforeRelease", this->LastTraceSceneId);
this->ReleaseOwnedMaterial(ownedAsset);
this->RecordTraceState("MatAfterRelease", this->LastTraceSceneId);
}
this->RecordTraceState("MatEnd", this->LastTraceSceneId);
}

void SceneManager::ReleaseOwnedModel(::RuntimeModel* ownedAsset)
{
    if (ownedAsset == nullptr)
    {
throw new ArgumentNullException("ownedAsset");
    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr)
    {
throw new InvalidOperationException("Runtime model release requires an initialized 3D render manager.");
    }
Core::Instance->RenderManager3D->ReleaseModel(ownedAsset);
}

void SceneManager::ReleaseOwnedModels(List<::RuntimeModel*>* ownedModels)
{
    if (ownedModels == nullptr)
    {
throw new ArgumentNullException("ownedModels");
    }
for (int32_t assetIndex = 0; assetIndex < ownedModels->get_Count(); assetIndex++) {
::RuntimeModel *ownedAsset = (*ownedModels).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (!this->ActiveOwnedModelReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
throw new InvalidOperationException("Scene-owned runtime model was not tracked before release.");
    }
    if (existingReferenceCount > 1)
    {
(*this->ActiveOwnedModelReferenceCounts).get_Item(ownedAsset) = existingReferenceCount - 1;
continue;
    }
this->ActiveOwnedModelReferenceCounts->Remove(ownedAsset);
this->ReleaseOwnedModel(ownedAsset);
}
}

void SceneManager::ReleaseOwnedTextures(List<::RuntimeTexture*>* ownedTextures)
{
    if (ownedTextures == nullptr)
    {
throw new ArgumentNullException("ownedTextures");
    }
for (int32_t assetIndex = 0; assetIndex < ownedTextures->get_Count(); assetIndex++) {
::RuntimeTexture *ownedAsset = (*ownedTextures).get_Item(static_cast<int32_t>(assetIndex));
    if (ownedAsset == nullptr)
    {
continue;
    }
int32_t existingReferenceCount;
    if (!this->ActiveOwnedTextureReferenceCounts->TryGetValue(ownedAsset, existingReferenceCount))
    {
throw new InvalidOperationException("Scene-owned runtime texture was not tracked before release.");
    }
    if (existingReferenceCount > 1)
    {
(*this->ActiveOwnedTextureReferenceCounts).get_Item(ownedAsset) = existingReferenceCount - 1;
continue;
    }
this->ActiveOwnedTextureReferenceCounts->Remove(ownedAsset);
this->ReleaseOwnedAsset(ownedAsset);
}
}

void SceneManager::ReleaseTransientSceneAsset(::SceneAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
Array<::SceneEntityAsset*> *rootEntities = asset->RootEntities;
Array<::SceneAssetReference*> *assetReferences = asset->AssetReferences;
::SceneSettingsAsset *sceneSettings = asset->get_SceneSettings();
asset->set_RootEntities(nullptr);
asset->set_AssetReferences(nullptr);
    if (rootEntities != nullptr)
    {
for (int32_t index = 0; index < rootEntities->get_Length(); index++) {
SceneManager::ReleaseTransientSceneEntityAsset((*rootEntities)[index]);
}
    }
    if (assetReferences != nullptr)
    {
for (int32_t index = 0; index < assetReferences->get_Length(); index++) {
delete (*assetReferences)[index];
}
    }
SceneManager::DeleteTransientArray<SceneEntityAsset*>(rootEntities);
SceneManager::DeleteTransientArray<SceneAssetReference*>(assetReferences);
SceneManager::ReleaseTransientSceneSettingsAsset(sceneSettings);
delete asset;
}

void SceneManager::ReleaseTransientSceneComponentAssetRecord(::SceneComponentAssetRecord* asset)
{
    if (asset == nullptr)
    {
return;    }
Array<uint8_t> *payload = asset->Payload;
asset->set_Payload(nullptr);
SceneManager::DeleteTransientArray<uint8_t>(payload);
asset->MarkReleasedForDiagnostics();
delete asset;
}

void SceneManager::ReleaseTransientSceneEntityAsset(::SceneEntityAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
Array<::SceneComponentAssetRecord*> *components = asset->Components;
Array<::SceneEntityPlatformExistenceOverrideAsset*> *platformExistenceOverrides = asset->PlatformExistenceOverrides;
Array<::SceneEntityPlatformTransformOverrideAsset*> *platformTransformOverrides = asset->PlatformTransformOverrides;
Array<::SceneEntityPlatformComponentOverrideAsset*> *platformComponentOverrides = asset->PlatformComponentOverrides;
Array<::SceneEntityAsset*> *children = asset->Children;
asset->set_Components(nullptr);
asset->set_PlatformExistenceOverrides(nullptr);
asset->set_PlatformTransformOverrides(nullptr);
asset->set_PlatformComponentOverrides(nullptr);
asset->set_Children(nullptr);
    if (components != nullptr)
    {
for (int32_t index = 0; index < components->get_Length(); index++) {
SceneManager::ReleaseTransientSceneComponentAssetRecord((*components)[index]);
}
    }
    if (platformExistenceOverrides != nullptr)
    {
for (int32_t index = 0; index < platformExistenceOverrides->get_Length(); index++) {
SceneManager::ReleaseTransientSceneEntityPlatformExistenceOverrideAsset((*platformExistenceOverrides)[index]);
}
    }
    if (platformTransformOverrides != nullptr)
    {
for (int32_t index = 0; index < platformTransformOverrides->get_Length(); index++) {
SceneManager::ReleaseTransientSceneEntityPlatformTransformOverrideAsset((*platformTransformOverrides)[index]);
}
    }
    if (platformComponentOverrides != nullptr)
    {
for (int32_t index = 0; index < platformComponentOverrides->get_Length(); index++) {
SceneManager::ReleaseTransientSceneEntityPlatformComponentOverrideAsset((*platformComponentOverrides)[index]);
}
    }
    if (children != nullptr)
    {
for (int32_t index = 0; index < children->get_Length(); index++) {
SceneManager::ReleaseTransientSceneEntityAsset((*children)[index]);
}
    }
SceneManager::DeleteTransientArray<SceneComponentAssetRecord*>(components);
SceneManager::DeleteTransientArray<SceneEntityPlatformExistenceOverrideAsset*>(platformExistenceOverrides);
SceneManager::DeleteTransientArray<SceneEntityPlatformTransformOverrideAsset*>(platformTransformOverrides);
SceneManager::DeleteTransientArray<SceneEntityPlatformComponentOverrideAsset*>(platformComponentOverrides);
SceneManager::DeleteTransientArray<SceneEntityAsset*>(children);
asset->MarkReleasedForDiagnostics();
delete asset;
}

void SceneManager::ReleaseTransientSceneEntityPlatformAddedComponentAsset(::SceneEntityPlatformAddedComponentAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
::SceneComponentAssetRecord *component = asset->Component;
asset->set_Component(nullptr);
SceneManager::ReleaseTransientSceneComponentAssetRecord(component);
delete asset;
}

void SceneManager::ReleaseTransientSceneEntityPlatformComponentOverrideAsset(::SceneEntityPlatformComponentOverrideAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
Array<std::string> *removedComponentKeys = asset->RemovedComponentKeys;
Array<::SceneEntityPlatformAddedComponentAsset*> *addedComponents = asset->AddedComponents;
asset->set_RemovedComponentKeys(nullptr);
asset->set_AddedComponents(nullptr);
    if (addedComponents != nullptr)
    {
for (int32_t index = 0; index < addedComponents->get_Length(); index++) {
SceneManager::ReleaseTransientSceneEntityPlatformAddedComponentAsset((*addedComponents)[index]);
}
    }
SceneManager::DeleteTransientArray<std::string>(removedComponentKeys);
SceneManager::DeleteTransientArray<SceneEntityPlatformAddedComponentAsset*>(addedComponents);
delete asset;
}

void SceneManager::ReleaseTransientSceneEntityPlatformExistenceOverrideAsset(::SceneEntityPlatformExistenceOverrideAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
delete asset;
}

void SceneManager::ReleaseTransientSceneEntityPlatformTransformOverrideAsset(::SceneEntityPlatformTransformOverrideAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
delete asset;
}

void SceneManager::ReleaseTransientSceneSettingsAsset(::SceneSettingsAsset* asset)
{
    if (asset == nullptr)
    {
return;    }
asset->ReleaseOwnedValuesForNativeDelete();
delete asset;
}

void SceneManager::ResetPhysicsTimingForSingleLoad()
{
    if (Core::Instance == nullptr)
    {
return;    }
Core::Instance->ResetPhysicsTimingState();
}

std::string SceneManager::ResolveSceneContentPath(std::string sceneId)
{
::RuntimeSceneCatalogEntry *entry;
    if (this->SceneCatalog != nullptr && this->SceneCatalog->TryGetEntry__out1(sceneId, entry))
    {
return entry->CookedRelativePath;    }
else {
    if (this->ScenePathResolver != nullptr)
    {
const std::string authoredScenePath = this->ScenePathResolver->ResolveScenePath(sceneId);
    if (String::IsNullOrWhiteSpace(authoredScenePath))
    {
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' resolved to an empty authored scene path."));
    }
return authoredScenePath;    }
}
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' was not found in the build scene catalog and no scene path resolver was configured."));
}

void SceneManager::UnloadAllScenes()
{
this->RecordTraceState("UnloadAllScenesBegin", String::Empty);
while (this->LoadedSceneRecords->get_Count() > 0) {
this->UnloadSceneImmediate((*this->LoadedSceneRecords).get_Item(static_cast<int32_t>(0))->SceneId);
}
this->RecordTraceState("UnloadAllScenesEnd", String::Empty);
}

void SceneManager::UnloadSceneImmediate(std::string sceneId)
{
this->RecordTraceState("UnloadSceneImmediateBegin", sceneId);
::LoadedSceneRecord* loadedSceneRecord;
    if (!this->LoadedSceneRecordsById->TryGetValue(sceneId, loadedSceneRecord))
    {
throw new InvalidOperationException(std::string("Runtime scene '") + sceneId + std::string("' is not currently loaded."));
    }
::SceneUnloadingEventArgs *sceneUnloadingEventArgs = new ::SceneUnloadingEventArgs(loadedSceneRecord->SceneId, loadedSceneRecord->CookedRelativePath, loadedSceneRecord->RootEntities);
{
auto __finallyGuard_0000016A = he_cpp_make_scope_exit([&]() {
delete sceneUnloadingEventArgs;
});
this->SceneUnloading.Invoke(this, sceneUnloadingEventArgs);
}
this->RecordTraceState("UnloadSceneImmediateBeforeDisposeSceneRoots", loadedSceneRecord->SceneId);
List<::Entity*> *releasedRootEntities = loadedSceneRecord->RootEntities;
::RuntimeSceneOwnedAssetSet *releasedOwnedAssets = loadedSceneRecord->OwnedAssets;
this->DisposeSceneRoots(releasedRootEntities);
this->RecordTraceState("UnloadSceneImmediateBeforeReleaseOwnedAssets", loadedSceneRecord->SceneId);
this->ReleaseOwnedAssets(releasedOwnedAssets);
this->RecordTraceState("UnloadSceneImmediateAfterReleaseOwnedAssets", loadedSceneRecord->SceneId);
this->RecordTraceState("UnloadSceneImmediateBeforeRemoveRecordById", loadedSceneRecord->SceneId);
this->LoadedSceneRecordsById->Remove(loadedSceneRecord->SceneId);
this->RecordTraceState("UnloadSceneImmediateBeforeRemoveRecord", loadedSceneRecord->SceneId);
this->LoadedSceneRecords->Remove(loadedSceneRecord);
this->RecordTraceState("UnloadSceneImmediateBeforeSceneUnloadedEvent", loadedSceneRecord->SceneId);
::SceneUnloadedEventArgs *sceneUnloadedEventArgs = new ::SceneUnloadedEventArgs(loadedSceneRecord->SceneId, loadedSceneRecord->CookedRelativePath);
{
auto __finallyGuard_0000016B = he_cpp_make_scope_exit([&]() {
delete sceneUnloadedEventArgs;
});
this->SceneUnloaded.Invoke(this, sceneUnloadedEventArgs);
}
this->RecordTraceState("UnloadSceneImmediateEnd", loadedSceneRecord->SceneId);
delete releasedRootEntities;
delete releasedOwnedAssets->OwnedTextures;
delete releasedOwnedAssets->OwnedFonts;
delete releasedOwnedAssets->OwnedModels;
delete releasedOwnedAssets->OwnedMaterials;
delete releasedOwnedAssets;
delete loadedSceneRecord;
}

void SceneManager::UnloadScenesForSingleLoad()
{
List<std::string> *sceneIdsToUnload = new List<std::string>();
auto __localDeleteGuard_0000016C = he_cpp_make_scope_exit([&]() {
delete sceneIdsToUnload;
});
for (int32_t index = 0; index < this->LoadedSceneRecords->get_Count(); index++) {
::LoadedSceneRecord *loadedSceneRecord = (*this->LoadedSceneRecords).get_Item(static_cast<int32_t>(index));
    if (!loadedSceneRecord->DontUnload)
    {
sceneIdsToUnload->Add(loadedSceneRecord->SceneId);
    }
}
for (int32_t index = 0; index < sceneIdsToUnload->get_Count(); index++) {
this->UnloadSceneImmediate((*sceneIdsToUnload).get_Item(static_cast<int32_t>(index)));
}
}

