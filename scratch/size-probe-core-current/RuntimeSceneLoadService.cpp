#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneLoadService.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "runtime/native_list.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "RuntimeSceneLoadResult.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "runtime/array.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "SceneEntityAsset.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "SceneEntityRuntimeIdComponent.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "Core.hpp"
#include "runtime/native_string.hpp"
#include "SceneAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeTexture.hpp"
#include "FontAsset.hpp"
#include "runtime/native_dictionary.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "AnimationClipAsset.hpp"
#include "SceneAssetReference.hpp"
#include "ModelAsset.hpp"
#include "TextureAsset.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "CoreInitializationOptions.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "int2.hpp"
#include "RenderManager2D.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "NativeOwnership.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"

const std::string& RuntimeSceneLoadService::get_LastTraceStage()
{
return this->LastTraceStage;
}

void RuntimeSceneLoadService::set_LastTraceStage(std::string value)
{
this->LastTraceStage = value;
}

int32_t RuntimeSceneLoadService::get_LastTraceRootEntityIndex()
{
return this->LastTraceRootEntityIndex;
}

void RuntimeSceneLoadService::set_LastTraceRootEntityIndex(int32_t value)
{
this->LastTraceRootEntityIndex = value;
}

int32_t RuntimeSceneLoadService::get_LastTraceEntityDepth()
{
return this->LastTraceEntityDepth;
}

void RuntimeSceneLoadService::set_LastTraceEntityDepth(int32_t value)
{
this->LastTraceEntityDepth = value;
}

const std::string& RuntimeSceneLoadService::get_LastTraceComponentTypeId()
{
return this->LastTraceComponentTypeId;
}

void RuntimeSceneLoadService::set_LastTraceComponentTypeId(std::string value)
{
this->LastTraceComponentTypeId = value;
}

const std::string& RuntimeSceneLoadService::get_LastTextLoadStage()
{
return this->ReferenceResolver->LastTextLoadStage;
}

const std::string& RuntimeSceneLoadService::get_LastTextFontRelativePath()
{
return this->ReferenceResolver->LastTextFontRelativePath;
}

const std::string& RuntimeSceneLoadService::get_LastTextureLoadStage()
{
return this->ReferenceResolver->LastTextureLoadStage;
}

const std::string& RuntimeSceneLoadService::get_LastTextureRelativePath()
{
return this->ReferenceResolver->LastTextureRelativePath;
}

const std::string& RuntimeSceneLoadService::get_LastFontDeserializeStage()
{
return this->ReferenceResolver->get_LastFontDeserializeStage();
}

List<::Entity*>* RuntimeSceneLoadService::Load(::SceneAsset* sceneAsset)
{
    if (sceneAsset == nullptr)
    {
throw new ArgumentNullException("sceneAsset");
    }
this->RecordTraceState("LoadBegin", static_cast<int32_t>(-1), static_cast<int32_t>(0), String::Empty);
Stopwatch *loadStopwatch = System::Diagnostics::Stopwatch::StartNew();
Array<::SceneEntityAsset*> *rootEntityAssets = ([&]() {
Array<::SceneEntityAsset*>* __coalesce_00000148 = sceneAsset->RootEntities;
return __coalesce_00000148 != nullptr ? __coalesce_00000148 : Array<SceneEntityAsset*>::Empty();
})();
List<::Entity*> *rootEntities = new List<::Entity*>(static_cast<int32_t>(rootEntityAssets->get_Length()));
{
auto __finallyGuard_00000149 = he_cpp_make_scope_exit([&]() {
delete loadStopwatch;
});
for (int32_t index = 0; index < rootEntityAssets->get_Length(); index++) {
this->RecordTraceState("BeforeRootEntityLoad", static_cast<int32_t>(index), static_cast<int32_t>(0), String::Empty);
rootEntities->Add(this->LoadEntity((*rootEntityAssets)[index], static_cast<int32_t>(index), static_cast<int32_t>(0)));
}
for (int32_t index = 0; index < rootEntities->get_Count(); index++) {
(*rootEntities).get_Item(static_cast<int32_t>(index))->InitializeHierarchy();
}
loadStopwatch->Stop();
this->RecordTraceState("LoadEnd", static_cast<int32_t>(rootEntities->get_Count() - 1), static_cast<int32_t>(0), String::Empty);
return rootEntities;}
}

::RuntimeSceneLoadResult* RuntimeSceneLoadService::LoadTracked(::SceneAsset* sceneAsset)
{
this->ReferenceResolver->BeginOwnedAssetTracking();
List<::Entity*> *rootEntities = this->Load(sceneAsset);
::RuntimeSceneOwnedAssetSet *ownedAssets = this->ReferenceResolver->CompleteOwnedAssetTracking();
return new ::RuntimeSceneLoadResult(rootEntities, ownedAssets);}

RuntimeSceneLoadService::RuntimeSceneLoadService(::RuntimeSceneAssetReferenceResolver* referenceResolver) : LastTraceStage(String::Empty), LastTraceRootEntityIndex(0), LastTraceEntityDepth(0), LastTraceComponentTypeId(String::Empty), ReferenceResolver(), ComponentRegistry()
{
this->ReferenceResolver = (referenceResolver != nullptr ? referenceResolver : throw new ArgumentNullException("referenceResolver"));
this->ComponentRegistry = RuntimeComponentRegistry::CreateDefault();
}

RuntimeSceneLoadService::RuntimeSceneLoadService(::RuntimeSceneAssetReferenceResolver* referenceResolver, ::RuntimeComponentRegistry* componentRegistry) : LastTraceStage(String::Empty), LastTraceRootEntityIndex(0), LastTraceEntityDepth(0), LastTraceComponentTypeId(String::Empty), ReferenceResolver(), ComponentRegistry()
{
this->ReferenceResolver = (referenceResolver != nullptr ? referenceResolver : throw new ArgumentNullException("referenceResolver"));
this->ComponentRegistry = (componentRegistry != nullptr ? componentRegistry : throw new ArgumentNullException("componentRegistry"));
}

::Component* RuntimeSceneLoadService::LoadComponent(::SceneComponentAssetRecord* record, int32_t rootEntityIndex, int32_t entityDepth)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
this->RecordTraceState("LoadComponentBegin", static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth), record->ComponentTypeId);
return this->ComponentRegistry->GetDeserializer(record->ComponentTypeId)->Deserialize(record, this->ReferenceResolver);}

::Entity* RuntimeSceneLoadService::LoadEntity(::SceneEntityAsset* entityAsset, int32_t rootEntityIndex, int32_t entityDepth)
{
    if (entityAsset == nullptr)
    {
throw new ArgumentNullException("entityAsset");
    }
this->RecordTraceState("LoadEntityBegin", static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth), String::Empty);
::Entity *entity = ([&]() {
auto __object_0000014A = new ::Entity();
__object_0000014A->set_Static(entityAsset->IsStatic);
__object_0000014A->set_Enabled(entityAsset->Enabled);
__object_0000014A->set_LayerMask(entityAsset->LayerMask);
__object_0000014A->set_LocalPosition(entityAsset->LocalPosition);
__object_0000014A->set_LocalScale(entityAsset->LocalScale);
__object_0000014A->set_LocalOrientation(entityAsset->LocalOrientation);
return __object_0000014A;
})();
entity->InitComponents();
entity->InitChildren();
    if (entityAsset->Id != 0u)
    {
entity->AddComponent(([&]() {
auto __object_0000014B = new ::SceneEntityRuntimeIdComponent();
__object_0000014B->set_SceneEntityId(entityAsset->Id);
return __object_0000014B;
})());
    }
Array<::SceneComponentAssetRecord*> *componentRecords = ([&]() {
Array<::SceneComponentAssetRecord*>* __coalesce_0000014C = entityAsset->Components;
return __coalesce_0000014C != nullptr ? __coalesce_0000014C : Array<SceneComponentAssetRecord*>::Empty();
})();
for (int32_t index = 0; index < componentRecords->get_Length(); index++) {
this->RecordTraceState("BeforeComponentLoad", static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth), (*componentRecords)[index] != nullptr ? (*componentRecords)[index]->ComponentTypeId : String::Empty);
entity->AddComponent(this->LoadComponent((*componentRecords)[index], static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth)));
}
Array<::SceneEntityAsset*> *childEntityAssets = ([&]() {
Array<::SceneEntityAsset*>* __coalesce_0000014D = entityAsset->Children;
return __coalesce_0000014D != nullptr ? __coalesce_0000014D : Array<SceneEntityAsset*>::Empty();
})();
for (int32_t index = 0; index < childEntityAssets->get_Length(); index++) {
this->RecordTraceState("BeforeChildEntityLoad", static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth + 1), String::Empty);
entity->AddChild(this->LoadEntity((*childEntityAssets)[index], static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth + 1)));
}
this->RecordTraceState("LoadEntityEnd", static_cast<int32_t>(rootEntityIndex), static_cast<int32_t>(entityDepth), String::Empty);
return entity;}

void RuntimeSceneLoadService::RecordTraceState(std::string stage, int32_t rootEntityIndex, int32_t entityDepth, std::string componentTypeId)
{
this->set_LastTraceStage(stage);
this->set_LastTraceRootEntityIndex(rootEntityIndex);
this->set_LastTraceEntityDepth(entityDepth);
this->set_LastTraceComponentTypeId(componentTypeId);
if (Core::Instance != nullptr)
{
Core::Instance->ReportSceneTransitionStage(std::string("SceneLoad:") + this->LastTraceStage);
}
}

