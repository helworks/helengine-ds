#ifdef DrawText
#undef DrawText
#endif
#include "LightComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "float4.hpp"
#include "ShadowMapMode.hpp"
#include "LightType.hpp"
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
#include "LightComponent.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/diagnostics/stopwatch.hpp"

::LightType LightComponent::get_LightType()
{
return this->LightType;
}

::float4 LightComponent::get_Color()
{
return this->Color;
}

void LightComponent::set_Color(::float4 value)
{
this->Color = value;
}

float LightComponent::get_Intensity()
{
return this->Intensity;
}

void LightComponent::set_Intensity(float value)
{
this->Intensity = value;
}

bool LightComponent::get_ShadowsEnabled()
{
return this->ShadowsEnabled;
}

void LightComponent::set_ShadowsEnabled(bool value)
{
this->ShadowsEnabled = value;
}

::ShadowMapMode LightComponent::get_ShadowMapMode()
{
return this->ShadowMapMode;
}

void LightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
this->ShadowMapMode = value;
}

float LightComponent::get_ShadowStrength()
{
return this->ShadowStrength;
}

void LightComponent::set_ShadowStrength(float value)
{
this->ShadowStrength = value;
}

void LightComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
this->RegisterWithObjectManager();
    }
}

void LightComponent::ComponentRemoved(::Entity* entity)
{
this->RemoveFromObjectManager();
Component::ComponentRemoved(entity);
}

void LightComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
this->RegisterWithObjectManager();
    }
else {
this->RemoveFromObjectManager();
}
}

::Entity* LightComponent::get_Parent()
{
return Component::get_Parent();
}

void LightComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool LightComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* LightComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool LightComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

LightComponent::LightComponent(::LightType lightType) : LightType(), Color(), Intensity(), ShadowsEnabled(), ShadowMapMode(), ShadowStrength()
{
this->LightType = lightType;
this->set_Color(::float4(1.0f, 1.0f, 1.0f, 1.0f));
this->set_Intensity(1.0f);
this->set_ShadowStrength(1.0f);
this->set_ShadowMapMode(ShadowMapMode::Auto);
}

void LightComponent::RegisterWithObjectManager()
{
    if (Core::Instance == nullptr || Core::Instance->ObjectManager == nullptr)
    {
throw new InvalidOperationException("Core object manager must exist before registering lights.");
    }
    DirectionalLightComponent* directionalLight = he_cpp_try_cast<DirectionalLightComponent>(this);
    if (directionalLight != nullptr)
    {
Core::Instance->ObjectManager->RegisterDirectionalLight(directionalLight);
    }
else {
    AmbientLightComponent* ambientLight = he_cpp_try_cast<AmbientLightComponent>(this);
    if (ambientLight != nullptr)
    {
Core::Instance->ObjectManager->RegisterAmbientLight(ambientLight);
    }
else {
    PointLightComponent* pointLight = he_cpp_try_cast<PointLightComponent>(this);
    if (pointLight != nullptr)
    {
Core::Instance->ObjectManager->RegisterPointLight(pointLight);
    }
else {
    SpotLightComponent* spotLight = he_cpp_try_cast<SpotLightComponent>(this);
    if (spotLight != nullptr)
    {
Core::Instance->ObjectManager->RegisterSpotLight(spotLight);
    }
else {
throw new InvalidOperationException("Unsupported light component type.");
}
}
}
}
}

void LightComponent::RemoveFromObjectManager()
{
    if (Core::Instance == nullptr || Core::Instance->ObjectManager == nullptr)
    {
throw new InvalidOperationException("Core object manager must exist before unregistering lights.");
    }
    DirectionalLightComponent* directionalLight = he_cpp_try_cast<DirectionalLightComponent>(this);
    if (directionalLight != nullptr)
    {
Core::Instance->ObjectManager->RemoveDirectionalLight(directionalLight);
    }
else {
    AmbientLightComponent* ambientLight = he_cpp_try_cast<AmbientLightComponent>(this);
    if (ambientLight != nullptr)
    {
Core::Instance->ObjectManager->RemoveAmbientLight(ambientLight);
    }
else {
    PointLightComponent* pointLight = he_cpp_try_cast<PointLightComponent>(this);
    if (pointLight != nullptr)
    {
Core::Instance->ObjectManager->RemovePointLight(pointLight);
    }
else {
    SpotLightComponent* spotLight = he_cpp_try_cast<SpotLightComponent>(this);
    if (spotLight != nullptr)
    {
Core::Instance->ObjectManager->RemoveSpotLight(spotLight);
    }
else {
throw new InvalidOperationException("Unsupported light component type.");
}
}
}
}
}

