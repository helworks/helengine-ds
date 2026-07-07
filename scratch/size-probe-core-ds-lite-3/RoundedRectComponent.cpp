#ifdef DrawText
#undef DrawText
#endif
#include "RoundedRectComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "RenderManager2D.hpp"
#include "int2.hpp"
#include "float4.hpp"
#include "byte4.hpp"
#include "RoundedRectCorners.hpp"
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
#include "RuntimeTexture.hpp"
#include "TextureAsset.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "RoundedRectComponent.hpp"
#include "system/diagnostics/stopwatch.hpp"

uint8_t RoundedRectComponent::get_RenderOrder2D()
{
return this->renderOrder2D;}

void RoundedRectComponent::set_RenderOrder2D(uint8_t value)
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

uint8_t RoundedRectComponent::get_LayerMask()
{
return this->LayerMask;
}

void RoundedRectComponent::set_LayerMask(uint8_t value)
{
this->LayerMask = value;
}

::RoundedRectCorners RoundedRectComponent::get_Corners()
{
return this->Corners;
}

void RoundedRectComponent::set_Corners(::RoundedRectCorners value)
{
this->Corners = value;
}

float RoundedRectComponent::get_Rotation()
{
return this->Rotation;
}

void RoundedRectComponent::set_Rotation(float value)
{
this->Rotation = value;
}

::byte4 RoundedRectComponent::get_Color()
{
return this->Color;
}

void RoundedRectComponent::set_Color(::byte4 value)
{
this->Color = value;
}

::float4 RoundedRectComponent::get_SourceRect()
{
return this->SourceRect;
}

void RoundedRectComponent::set_SourceRect(::float4 value)
{
this->SourceRect = value;
}

::int2 RoundedRectComponent::get_Size()
{
return this->Size;
}

void RoundedRectComponent::set_Size(::int2 value)
{
this->Size = value;
}

::int2 RoundedRectComponent::get_AnchorSize()
{
return this->Size;
}

float RoundedRectComponent::get_Radius()
{
return this->Radius;
}

void RoundedRectComponent::set_Radius(float value)
{
this->Radius = value;
}

float RoundedRectComponent::get_BorderThickness()
{
return this->BorderThickness;
}

void RoundedRectComponent::set_BorderThickness(float value)
{
this->BorderThickness = value;
}

::byte4 RoundedRectComponent::get_FillColor()
{
return this->FillColor;
}

void RoundedRectComponent::set_FillColor(::byte4 value)
{
this->FillColor = value;
}

::byte4 RoundedRectComponent::get_BorderColor()
{
return this->BorderColor;
}

void RoundedRectComponent::set_BorderColor(::byte4 value)
{
this->BorderColor = value;
}

void RoundedRectComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
}

void RoundedRectComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::Instance->ObjectManager->RemoveFromRender2D(this);
}

void RoundedRectComponent::Draw()
{
Core::Instance->RenderManager2D->DrawRoundedRect(this);
}

void RoundedRectComponent::ParentEnabledChange(bool newEnabled)
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

RoundedRectComponent::RoundedRectComponent() : LayerMask(), Corners(), Rotation(), Color(), SourceRect(), Size(), Radius(), BorderThickness(), FillColor(), BorderColor(), renderOrder2D()
{
this->set_Size(::int2(static_cast<int32_t>(64), static_cast<int32_t>(32)));
this->set_SourceRect(::float4(0.0f, 0.0f, 1.0f, 1.0f));
this->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
this->set_Radius(8.0f);
this->set_BorderThickness(0.0f);
this->set_FillColor(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
this->set_BorderColor(::byte4(static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(255)));
this->set_Corners(RoundedRectCorners::All);
}

::Entity* RoundedRectComponent::get_Parent()
{
return Component::get_Parent();
}

void RoundedRectComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool RoundedRectComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* RoundedRectComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool RoundedRectComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

