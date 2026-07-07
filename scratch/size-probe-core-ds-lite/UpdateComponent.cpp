#ifdef DrawText
#undef DrawText
#endif
#include "UpdateComponent.hpp"
#include "Component.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "ComponentExecutionPolicy.hpp"
#include "Entity.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
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
#include "runtime/native_list.hpp"
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
#include "UpdateComponent.hpp"
#include "system/diagnostics/stopwatch.hpp"

UpdateComponent::UpdateComponent() : updateOrder()
{
}

uint8_t UpdateComponent::get_UpdateOrder()
{
return this->updateOrder;}

void UpdateComponent::set_UpdateOrder(uint8_t value)
{
    if (this->updateOrder != value)
    {
    if (this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled() && ComponentExecutionPolicy::ShouldRunComponentLifecycle(this, this->Parent))
    {
Core::Instance->ObjectManager->RemoveFromUpdate(this, static_cast<uint8_t>(this->updateOrder));
this->updateOrder = value;
Core::Instance->ObjectManager->RegisterForUpdate(this);
    }
else {
this->updateOrder = value;
}
    }
}

void UpdateComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled() && ComponentExecutionPolicy::ShouldRunComponentLifecycle(this, entity))
    {
Core::Instance->ObjectManager->RegisterForUpdate(this);
    }
}

void UpdateComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!ComponentExecutionPolicy::ShouldRunComponentLifecycle(this, this->Parent))
    {
return;    }
    if (newEnabled)
    {
Core::Instance->ObjectManager->RegisterForUpdate(this);
    }
else {
Core::Instance->ObjectManager->RemoveFromUpdate(this, static_cast<uint8_t>(this->updateOrder));
}
}

void UpdateComponent::Update()
{
}

::Entity* UpdateComponent::get_Parent()
{
return Component::get_Parent();
}

void UpdateComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool UpdateComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* UpdateComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool UpdateComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

