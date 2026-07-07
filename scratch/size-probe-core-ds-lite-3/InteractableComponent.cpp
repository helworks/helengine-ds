#ifdef DrawText
#undef DrawText
#endif
#include "InteractableComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "PointerInteractionSystem.hpp"
#include "PointerCursorKind.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "PointerInteraction.hpp"
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
#include "RenderManager2D.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
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
#include "system/action.hpp"
#include "InteractableComponent.hpp"
#include "runtime/native_event.hpp"
#include "system/diagnostics/stopwatch.hpp"

InteractableComponent::InteractableComponent() : HoverCursor(), Size(), CursorEvent()
{
}

::PointerCursorKind InteractableComponent::get_HoverCursor()
{
return this->HoverCursor;
}

void InteractableComponent::set_HoverCursor(::PointerCursorKind value)
{
this->HoverCursor = value;
}

::int2 InteractableComponent::get_Size()
{
return this->Size;
}

void InteractableComponent::set_Size(::int2 value)
{
this->Size = value;
}

void InteractableComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RegisterInteractable(this);
    }
}

void InteractableComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::Instance->ObjectManager->RemoveInteractable(this);
    if (Core::Instance != nullptr && Core::Instance->PointerInteractionSystem != nullptr)
    {
Core::Instance->PointerInteractionSystem->ClearInteractionFor(this);
    }
}

void InteractableComponent::OnCursor(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
this->CursorEvent.Invoke(relPos, delta, state);
}

void InteractableComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::Instance->ObjectManager->RegisterInteractable(this);
    }
else {
Core::Instance->ObjectManager->RemoveInteractable(this);
    if (Core::Instance != nullptr && Core::Instance->PointerInteractionSystem != nullptr)
    {
Core::Instance->PointerInteractionSystem->ClearInteractionFor(this);
    }
}
}

::Entity* InteractableComponent::get_Parent()
{
return this->Component::get_Parent();
}

void InteractableComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool InteractableComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* InteractableComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool InteractableComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

