#ifdef DrawText
#undef DrawText
#endif
#include "CameraComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "CoreInitializationOptions.hpp"
#include "Core.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "ObjectManager.hpp"
#include "NativeOwnership.hpp"
#include "CameraProjectionUtils.hpp"
#include "CameraRenderSettings.hpp"
#include "float4.hpp"
#include "CameraClearSettings.hpp"
#include "RenderList2D.hpp"
#include "RenderList3D.hpp"
#include "runtime/native_event.hpp"
#include "RenderTarget.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeDiagnosticsProvider.hpp"
#include "StandardPlatformInputConfiguration.hpp"
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
#include "runtime/native_dictionary.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "float3.hpp"
#include "float4x4.hpp"
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
#include "runtime/array.hpp"
#include "DepthPrepassMode.hpp"
#include "PostProcessTier.hpp"
#include "IRenderVisitor2D.hpp"
#include "IRenderVisitor3D.hpp"
#include "CameraComponent.hpp"
#include "system/action.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/diagnostics/stopwatch.hpp"

uint8_t CameraComponent::get_CameraDrawOrder()
{
return this->cameraDrawOrder;}

void CameraComponent::set_CameraDrawOrder(uint8_t value)
{
    if (this->cameraDrawOrder != value)
    {
    if (this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RemoveCamera(this);
this->cameraDrawOrder = value;
Core::Instance->ObjectManager->RegisterCamera(this);
    }
else {
this->cameraDrawOrder = value;
}
    }
}

::float4 CameraComponent::get_Viewport()
{
return this->viewportValue;}

void CameraComponent::set_Viewport(::float4 value)
{
    if (this->viewportValue.X != value.X || this->viewportValue.Y != value.Y || this->viewportValue.Z != value.Z || this->viewportValue.W != value.W)
    {
this->viewportValue = value;
this->RaiseViewportChanged();
    }
}

float CameraComponent::get_NearPlaneDistance()
{
return this->NearPlaneDistanceValue;}

void CameraComponent::set_NearPlaneDistance(float value)
{
this->NearPlaneDistanceValue = CameraProjectionUtils::ClampNearPlaneDistance(value, this->FarPlaneDistanceValue);
this->FarPlaneDistanceValue = CameraProjectionUtils::ClampFarPlaneDistance(this->NearPlaneDistanceValue, this->FarPlaneDistanceValue);
}

float CameraComponent::get_FarPlaneDistance()
{
return this->FarPlaneDistanceValue;}

void CameraComponent::set_FarPlaneDistance(float value)
{
this->FarPlaneDistanceValue = CameraProjectionUtils::ClampFarPlaneDistance(this->NearPlaneDistanceValue, value);
}

::RenderTarget* CameraComponent::get_RenderTarget()
{
return this->RenderTarget;
}

void CameraComponent::set_RenderTarget(::RenderTarget* value)
{
this->RenderTarget = value;
}

::CameraClearSettings CameraComponent::get_ClearSettings()
{
return this->ClearSettings;
}

void CameraComponent::set_ClearSettings(::CameraClearSettings value)
{
this->ClearSettings = value;
}

::CameraRenderSettings* CameraComponent::get_RenderSettings()
{
return this->RenderSettingsValue;}

void CameraComponent::set_RenderSettings(::CameraRenderSettings* value)
{
::CameraRenderSettings *newValue = (value != nullptr ? value : throw new ArgumentNullException("value"));
    if (this->RenderSettingsValue != nullptr && !(this->RenderSettingsValue == newValue))
    {
delete this->RenderSettingsValue;
    }
this->RenderSettingsValue = newValue;
}

::IRenderQueue2D* CameraComponent::get_RenderQueue2D()
{
return this->renderList2D;}

::IRenderQueue3D* CameraComponent::get_RenderQueue3D()
{
return this->renderList3D;}

uint16_t CameraComponent::get_LayerMask()
{
return this->layerMask;}

void CameraComponent::set_LayerMask(uint16_t value)
{
    if (this->layerMask != value)
    {
    if (this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RemoveCamera(this);
this->layerMask = value;
Core::Instance->ObjectManager->RegisterCamera(this);
    }
else {
this->layerMask = value;
}
    }
}

CameraComponent::CameraComponent() : ViewportChanged(), RenderTarget(), ClearSettings(), cameraDrawOrder(), layerMask(), viewportValue(), RenderSettingsValue(), NearPlaneDistanceValue(), FarPlaneDistanceValue(), renderList2D(), renderList3D()
{
this->set_LayerMask(0b11111111);
this->viewportValue = ::float4(0.0f, 0.0f, 1.0f, 1.0f);
this->NearPlaneDistanceValue = 0.1f;
this->FarPlaneDistanceValue = 100.0f;
this->set_ClearSettings(([&]() {
auto __ctor_arg_000001DB = true;
auto __ctor_arg_000001DC = ::float4(0.0f, 0.0f, 0.0f, 0.0f);
auto __ctor_arg_000001DD = true;
auto __ctor_arg_000001DE = 1.0f;
auto __ctor_arg_000001DF = false;
auto __ctor_arg_000001E0 = static_cast<uint8_t>(0);
return ::CameraClearSettings(__ctor_arg_000001DB, __ctor_arg_000001DC, __ctor_arg_000001DD, __ctor_arg_000001DE, __ctor_arg_000001DF, __ctor_arg_000001E0);
})());
this->set_RenderSettings(new ::CameraRenderSettings());
this->InitializeLists();
}

void CameraComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RegisterCamera(this);
    }
}

void CameraComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::Instance->ObjectManager->RemoveCamera(this);
}

void CameraComponent::Dispose()
{
if (this->renderList2D != nullptr)
{
this->renderList2D->Dispose();
delete this->renderList2D;
}
if (this->renderList3D != nullptr)
{
this->renderList3D->Dispose();
delete this->renderList3D;
}
delete this->RenderSettingsValue;
this->renderList2D = nullptr;
this->renderList3D = nullptr;
this->RenderSettingsValue = nullptr;
this->set_RenderTarget(nullptr);
Component::Dispose();
}

void CameraComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::Instance->ObjectManager->RegisterCamera(this);
    }
else {
Core::Instance->ObjectManager->RemoveCamera(this);
}
}

::Entity* CameraComponent::get_Parent()
{
return this->Component::get_Parent();
}

void CameraComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool CameraComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* CameraComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool CameraComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void CameraComponent::InitializeLists()
{
    if (Core::Instance == nullptr || Core::Instance->InitializationOptions == nullptr)
    {
throw new InvalidOperationException("Core initialization options must be set before creating camera lists.");
    }
::CoreInitializationOptions *settings = Core::Instance->InitializationOptions;
settings->Normalize();
this->renderList2D = new ::RenderList2D(static_cast<int32_t>(settings->RenderList2DInitialCapacity));
this->renderList3D = new ::RenderList3D(static_cast<int32_t>(settings->RenderList3DInitialCapacity));
}

void CameraComponent::RaiseViewportChanged()
{
    if (true)
    {
this->ViewportChanged.Invoke();
    }
}

