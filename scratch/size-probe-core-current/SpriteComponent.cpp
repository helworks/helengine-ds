#ifdef DrawText
#undef DrawText
#endif
#include "SpriteComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "RenderManager2D.hpp"
#include "float4.hpp"
#include "byte4.hpp"
#include "RuntimeTexture.hpp"
#include "int2.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "FontAsset.hpp"
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
#include "TextureAsset.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "SpriteComponent.hpp"
#include "system/diagnostics/stopwatch.hpp"

uint8_t SpriteComponent::get_RenderOrder2D()
{
return this->renderOrder2D;}

void SpriteComponent::set_RenderOrder2D(uint8_t value)
{
    if (this->renderOrder2D != value)
    {
    if (this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RemoveFromRender2D(this);
this->renderOrder2D = value;
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
else {
this->renderOrder2D = value;
}
    }
}

::RuntimeTexture* SpriteComponent::get_Texture()
{
return this->Texture;
}

void SpriteComponent::set_Texture(::RuntimeTexture* value)
{
this->Texture = value;
}

uint8_t SpriteComponent::get_LayerMask()
{
return this->LayerMask;
}

void SpriteComponent::set_LayerMask(uint8_t value)
{
this->LayerMask = value;
}

::float4 SpriteComponent::get_SourceRect()
{
return this->SourceRect;
}

void SpriteComponent::set_SourceRect(::float4 value)
{
this->SourceRect = value;
}

::int2 SpriteComponent::get_Size()
{
return this->Size;
}

void SpriteComponent::set_Size(::int2 value)
{
this->Size = value;
}

::int2 SpriteComponent::get_AnchorSize()
{
return this->Size;
}

::byte4 SpriteComponent::get_Color()
{
return this->Color;
}

void SpriteComponent::set_Color(::byte4 value)
{
this->Color = value;
}

void SpriteComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
}

void SpriteComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::Instance->ObjectManager->RemoveFromRender2D(this);
}

void SpriteComponent::Draw()
{
Core::Instance->RenderManager2D->DrawSprite(this);
}

void SpriteComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
else {
Core::Instance->ObjectManager->RemoveFromRender2D(this);
}
}

SpriteComponent::SpriteComponent() : Texture(), LayerMask(), SourceRect(), Size(), Color(), renderOrder2D()
{
this->set_SourceRect(::float4(0.0f, 0.0f, 1.0f, 1.0f));
this->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
}

::Entity* SpriteComponent::get_Parent()
{
return Component::get_Parent();
}

void SpriteComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool SpriteComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* SpriteComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool SpriteComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

