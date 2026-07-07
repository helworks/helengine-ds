#ifdef DrawText
#undef DrawText
#endif
#include "MeshComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "NativeOwnership.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "FontAsset.hpp"
#include "int2.hpp"
#include "RenderManager2D.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
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
#include "IUpdateable.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "ICamera.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "IInteractable2D.hpp"
#include "PendingUpdateOperation.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "MeshComponent.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/diagnostics/stopwatch.hpp"

::RuntimeModel* MeshComponent::get_Model()
{
return this->Model;
}

void MeshComponent::set_Model(::RuntimeModel* value)
{
this->Model = value;
}

Array<::RuntimeMaterial*>* MeshComponent::get_Materials()
{
return this->MaterialsBySlot;}

void MeshComponent::set_Materials(Array<::RuntimeMaterial*>* value)
{
    if (value == nullptr)
    {
throw new ArgumentNullException("value");
    }
Array<::RuntimeMaterial*> *previousMaterials = this->MaterialsBySlot;
this->MaterialsBySlot = new Array<RuntimeMaterial*>(value->get_Length());
Array<RuntimeMaterial*>::Copy(value, this->MaterialsBySlot, value->get_Length());
delete previousMaterials;
previousMaterials = nullptr;
}

uint8_t MeshComponent::get_RenderOrder3D()
{
return this->renderOrder3D;}

void MeshComponent::set_RenderOrder3D(uint8_t value)
{
    if (this->renderOrder3D != value)
    {
    if (this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RemoveFromRender3D(this);
this->renderOrder3D = value;
Core::Instance->ObjectManager->RegisterForRender3D(this);
    }
else {
this->renderOrder3D = value;
}
    }
}

void MeshComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RegisterForRender3D(this);
    }
}

void MeshComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::Instance->ObjectManager->RemoveFromRender3D(this);
}

void MeshComponent::Dispose()
{
delete this->MaterialsBySlot;
this->MaterialsBySlot = nullptr;
this->set_Model(nullptr);
Component::Dispose();
}

MeshComponent::MeshComponent() : Model(), renderOrder3D(), MaterialsBySlot()
{
this->MaterialsBySlot = new Array<RuntimeMaterial*>(0);
}

void MeshComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::Instance->ObjectManager->RegisterForRender3D(this);
    }
else {
Core::Instance->ObjectManager->RemoveFromRender3D(this);
}
}

void MeshComponent::SetMaterials(Array<::RuntimeMaterial*>* runtimeMaterials)
{
this->set_Materials(runtimeMaterials);
}

::Entity* MeshComponent::get_Parent()
{
return this->Component::get_Parent();
}

void MeshComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool MeshComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* MeshComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool MeshComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

