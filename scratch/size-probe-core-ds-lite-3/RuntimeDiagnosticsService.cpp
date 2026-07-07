#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "runtime/native_list.hpp"
#include "SceneManager.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeMemoryCounters.hpp"
#include "IRuntimeMemoryCounterProvider.hpp"
#include "IRuntimeDiagnosticsProvider.hpp"
#include "ICamera.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "Entity.hpp"
#include "RuntimeDiagnosticsMetric.hpp"
#include "ObjectManager.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_event.hpp"
#include "LoadedSceneRecord.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "ContentManager.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "runtime/native_dictionary.hpp"
#include "PendingSceneOperation.hpp"
#include "RuntimeTexture.hpp"
#include "FontAsset.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/array.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "SceneAsset.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "float4.hpp"
#include "RenderTarget.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "IDrawable2D.hpp"
#include "IRenderVisitor2D.hpp"
#include "IDrawable3D.hpp"
#include "IRenderVisitor3D.hpp"
#include "float3.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "IUpdateable.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "IInteractable2D.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"

void RuntimeDiagnosticsService::CaptureMemoryCounters(::RuntimeMemoryCounters* counters)
{
    if (counters == nullptr)
    {
throw new ArgumentNullException("counters");
    }
counters->Reset();
    IRuntimeMemoryCounterProvider* memoryCounterProvider = he_cpp_try_cast<IRuntimeMemoryCounterProvider>(this->RuntimeDiagnosticsProvider);
    if (memoryCounterProvider != nullptr)
    {
memoryCounterProvider->CaptureMemoryCounters(counters);
return;    }
    if (this->RuntimeDiagnosticsProvider == nullptr)
    {
return;    }
::RuntimeMemoryDiagnosticsSnapshot *snapshot = this->RuntimeDiagnosticsProvider->CaptureSnapshot();
{
auto __finallyGuard_00000135 = he_cpp_make_scope_exit([&]() {
if (snapshot != nullptr)
{
snapshot->Dispose();
delete snapshot;
}
});
    if (snapshot != nullptr)
    {
counters->CopyFromSnapshot(snapshot);
    }
}
}

::RuntimeMemoryDiagnosticsSnapshot* RuntimeDiagnosticsService::CaptureSnapshot()
{
::RuntimeMemoryDiagnosticsSnapshot *snapshot = this->RuntimeDiagnosticsProvider != nullptr ? this->RuntimeDiagnosticsProvider->CaptureSnapshot() : new ::RuntimeMemoryDiagnosticsSnapshot();
    if (snapshot == nullptr)
    {
snapshot = new ::RuntimeMemoryDiagnosticsSnapshot();
    }
List<std::string> *trackedSceneIds = ([&]() {
List<std::string>* __coalesce_00000136 = snapshot->TrackedSceneIds;
return __coalesce_00000136 != nullptr ? __coalesce_00000136 : new List<std::string>();
})();
trackedSceneIds->Clear();
    if (this->RuntimeSceneManager != nullptr)
    {
List<std::string> *loadedSceneIds = this->RuntimeSceneManager->GetLoadedSceneIds();
for (int32_t index = 0; index < loadedSceneIds->get_Count(); index++) {
trackedSceneIds->Add((*loadedSceneIds).get_Item(static_cast<int32_t>(index)));
}
delete loadedSceneIds;
    }
snapshot->set_TrackedSceneIds(trackedSceneIds);
this->AppendEngineCollectionMetrics(snapshot);
return snapshot;}

RuntimeDiagnosticsService::RuntimeDiagnosticsService(::IRuntimeDiagnosticsProvider* runtimeDiagnosticsProvider, ::SceneManager* runtimeSceneManager, ::ObjectManager* runtimeObjectManager) : RuntimeDiagnosticsProvider(), RuntimeSceneManager(), RuntimeObjectManager()
{
this->RuntimeDiagnosticsProvider = runtimeDiagnosticsProvider;
this->RuntimeSceneManager = runtimeSceneManager;
this->RuntimeObjectManager = runtimeObjectManager;
}

void RuntimeDiagnosticsService::AppendCameraQueueMetrics(::RuntimeMemoryDiagnosticsSnapshot* snapshot)
{
int32_t totalRenderList2DCount = 0;
int32_t totalRenderList2DCapacity = 0;
int32_t totalRenderList3DCount = 0;
int32_t totalRenderList3DCapacity = 0;
for (int32_t cameraIndex = 0; cameraIndex < this->RuntimeObjectManager->Cameras->get_Count(); cameraIndex++) {
::ICamera *camera = (*this->RuntimeObjectManager->Cameras).get_Item(static_cast<int32_t>(cameraIndex));
    if (camera == nullptr)
    {
continue;
    }
::IRenderQueue2D *renderQueue2D = camera->get_RenderQueue2D();
    if (renderQueue2D != nullptr)
    {
totalRenderList2DCount += renderQueue2D->get_Count();
totalRenderList2DCapacity += renderQueue2D->get_Capacity();
    }
::IRenderQueue3D *renderQueue3D = camera->get_RenderQueue3D();
    if (renderQueue3D != nullptr)
    {
totalRenderList3DCount += renderQueue3D->get_Count();
totalRenderList3DCapacity += renderQueue3D->get_Capacity();
    }
}
RuntimeDiagnosticsService::AppendMetric(snapshot, "camera_render_list_2d_count_total", static_cast<uint64_t>(static_cast<uint64_t>(totalRenderList2DCount)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "camera_render_list_2d_capacity_total", static_cast<uint64_t>(static_cast<uint64_t>(totalRenderList2DCapacity)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "camera_render_list_2d_estimated_bytes_total", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(totalRenderList2DCapacity))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "camera_render_list_3d_count_total", static_cast<uint64_t>(static_cast<uint64_t>(totalRenderList3DCount)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "camera_render_list_3d_capacity_total", static_cast<uint64_t>(static_cast<uint64_t>(totalRenderList3DCapacity)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "camera_render_list_3d_estimated_bytes_total", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(totalRenderList3DCapacity))));
}

void RuntimeDiagnosticsService::AppendEngineCollectionMetrics(::RuntimeMemoryDiagnosticsSnapshot* snapshot)
{
    if (snapshot == nullptr)
    {
throw new ArgumentNullException("snapshot");
    }
    if (this->RuntimeObjectManager != nullptr)
    {
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_entities_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->Entities->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_entities_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_EntityCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_entities_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_EntityCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_updateables_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->Updateables->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_updateables_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_UpdateableCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_updateables_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_UpdateableCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_pending_update_operations_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_PendingUpdateOperationCount())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_pending_update_operations_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_PendingUpdateOperationCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_pending_update_operations_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_PendingUpdateOperationCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_drawables_2d_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->Drawables2D->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_drawables_2d_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_Drawable2DCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_drawables_2d_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_Drawable2DCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_drawables_3d_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->Drawables3D->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_drawables_3d_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_Drawable3DCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_drawables_3d_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_Drawable3DCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_cameras_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->Cameras->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_cameras_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_CameraCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_cameras_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_CameraCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_directional_lights_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->DirectionalLights->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_ambient_lights_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->AmbientLights->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_point_lights_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->PointLights->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_spot_lights_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->SpotLights->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_interactables_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->Interactables->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_interactables_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeObjectManager->get_InteractableCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "object_manager_interactables_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeObjectManager->get_InteractableCapacity()))));
this->AppendEntityHierarchyMetrics(snapshot);
this->AppendCameraQueueMetrics(snapshot);
    }
    if (this->RuntimeSceneManager != nullptr)
    {
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_loaded_scenes_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_LoadedScenes()->get_Count())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_loaded_scenes_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_LoadedSceneRecordCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_loaded_scenes_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeSceneManager->get_LoadedSceneRecordCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_pending_operations_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->LastTracePendingOperationCount)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_pending_operations_capacity", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_PendingOperationCapacity())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_pending_operations_estimated_bytes", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(this->RuntimeSceneManager->get_PendingOperationCapacity()))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_active_owned_textures_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_ActiveOwnedTextureReferenceCount())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_active_owned_fonts_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_ActiveOwnedFontReferenceCount())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_active_owned_models_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_ActiveOwnedModelReferenceCount())));
RuntimeDiagnosticsService::AppendMetric(snapshot, "scene_manager_active_owned_materials_count", static_cast<uint64_t>(static_cast<uint64_t>(this->RuntimeSceneManager->get_ActiveOwnedMaterialReferenceCount())));
    }
}

void RuntimeDiagnosticsService::AppendEntityHierarchyMetrics(::RuntimeMemoryDiagnosticsSnapshot* snapshot)
{
int32_t totalComponentCount = 0;
int32_t totalComponentCapacity = 0;
int32_t totalChildCount = 0;
int32_t totalChildCapacity = 0;
for (int32_t entityIndex = 0; entityIndex < this->RuntimeObjectManager->Entities->get_Count(); entityIndex++) {
::Entity *entity = (*this->RuntimeObjectManager->Entities).get_Item(static_cast<int32_t>(entityIndex));
    if (entity == nullptr)
    {
continue;
    }
    if (entity->get_Components() != nullptr)
    {
totalComponentCount += entity->get_Components()->get_Count();
totalComponentCapacity += entity->get_Components()->get_Capacity();
    }
    if (entity->get_Children() != nullptr)
    {
totalChildCount += entity->get_Children()->get_Count();
totalChildCapacity += entity->get_Children()->get_Capacity();
    }
}
RuntimeDiagnosticsService::AppendMetric(snapshot, "entity_components_count_total", static_cast<uint64_t>(static_cast<uint64_t>(totalComponentCount)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "entity_components_capacity_total", static_cast<uint64_t>(static_cast<uint64_t>(totalComponentCapacity)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "entity_components_estimated_bytes_total", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(totalComponentCapacity))));
RuntimeDiagnosticsService::AppendMetric(snapshot, "entity_children_count_total", static_cast<uint64_t>(static_cast<uint64_t>(totalChildCount)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "entity_children_capacity_total", static_cast<uint64_t>(static_cast<uint64_t>(totalChildCapacity)));
RuntimeDiagnosticsService::AppendMetric(snapshot, "entity_children_estimated_bytes_total", static_cast<uint64_t>(RuntimeDiagnosticsService::EstimateReferenceListBytes(static_cast<int32_t>(totalChildCapacity))));
}

void RuntimeDiagnosticsService::AppendMetric(::RuntimeMemoryDiagnosticsSnapshot* snapshot, std::string name, uint64_t value)
{
snapshot->DetailMetrics->Add(new ::RuntimeDiagnosticsMetric(name, static_cast<uint64_t>(value)));
}

uint64_t RuntimeDiagnosticsService::EstimateReferenceListBytes(int32_t capacity)
{
    if (capacity <= 0)
    {
return 0;    }
return static_cast<uint64_t>(capacity) * ReferenceSlotSizeInBytes;}

