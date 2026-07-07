#ifdef DrawText
#undef DrawText
#endif
#include "SceneMapComponent.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "UpdateComponent.hpp"
#include "Core.hpp"
#include "SceneManager.hpp"
#include "SceneMapComponent.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/native_dictionary.hpp"
#include "Entity.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
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
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_list.hpp"
#include "LoadedSceneRecord.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "PendingSceneOperation.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
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
#include "Component.hpp"
#include "system/string_comparer.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/string_comparer.hpp"

::SceneMapComponent* SceneMapComponent::Instance;

::SceneMapComponent* SceneMapComponent::get_Instance()
{
return SceneMapComponent::Instance;
}

void SceneMapComponent::set_Instance(::SceneMapComponent* value)
{
SceneMapComponent::Instance = value;
}

const std::string& SceneMapComponent::get_InitialSceneId()
{
return this->InitialSceneId;
}

void SceneMapComponent::set_InitialSceneId(std::string value)
{
this->InitialSceneId = value;
}

Dictionary<std::string, std::string>* SceneMapComponent::get_Mappings()
{
return this->MappingsValue;
}

void SceneMapComponent::set_Mappings(Dictionary<std::string, std::string>* value)
{
    if (value == nullptr)
    {
throw new ArgumentNullException("value");
    }
this->MappingsValue = value;
}

void SceneMapComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
this->RegisterSingleton();
}

void SceneMapComponent::ComponentRemoved(::Entity* entity)
{
this->ClearSingletonIfOwned();
UpdateComponent::ComponentRemoved(entity);
}

void SceneMapComponent::Dispose()
{
this->ClearSingletonIfOwned();
UpdateComponent::Dispose();
}

std::string SceneMapComponent::ResolveSceneId(std::string sceneId)
{
std::string mappedSceneId;
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_000001D4 = "Scene id must be provided.";
auto __ctor_arg_000001D5 = "sceneId";
return new ArgumentException(__ctor_arg_000001D4, __ctor_arg_000001D5);
})();
    }
else {
    if (Instance == nullptr)
    {
return sceneId;    }
else {
    if (Instance->get_Mappings()->TryGetValue(sceneId, mappedSceneId) && !String::IsNullOrWhiteSpace(mappedSceneId))
    {
return mappedSceneId;    }
}
}
return sceneId;}

SceneMapComponent::SceneMapComponent() : InitialSceneId(), MappingsValue()
{
this->MappingsValue = new Dictionary<std::string, std::string>(StringComparer::get_Ordinal());
this->set_InitialSceneId(String::Empty);
}

void SceneMapComponent::Update()
{
UpdateComponent::Update();
this->RequestInitialSceneLoad();
}

uint8_t SceneMapComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void SceneMapComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* SceneMapComponent::get_Parent()
{
return Component::get_Parent();
}

void SceneMapComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool SceneMapComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* SceneMapComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool SceneMapComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

bool SceneMapComponent::StartupSceneWasRequested;

void SceneMapComponent::ClearSingletonIfOwned()
{
    if ((Instance == this))
    {
SceneMapComponent::set_Instance(nullptr);
    }
}

void SceneMapComponent::RegisterSingleton()
{
    if (Instance == nullptr)
    {
SceneMapComponent::set_Instance(this);
    }
else {
    if (!(Instance == this))
    {
throw new InvalidOperationException("Only one active SceneMapComponent may exist at a time.");
    }
}
}

void SceneMapComponent::RequestInitialSceneLoad()
{
    if (StartupSceneWasRequested)
    {
return;    }
else {
    if (String::IsNullOrWhiteSpace(this->InitialSceneId))
    {
return;    }
else {
    if (Core::Instance == nullptr || Core::Instance->SceneManager == nullptr)
    {
throw new InvalidOperationException("SceneMapComponent startup redirection requires an initialized SceneManager.");
    }
}
}
const std::string resolvedSceneId = SceneMapComponent::ResolveSceneId(this->InitialSceneId);
StartupSceneWasRequested = true;
Core::Instance->SceneManager->LoadScene(resolvedSceneId, static_cast<SceneLoadMode>(SceneLoadMode::Single));
}

