#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeComponentRegistry.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_type.hpp"
#include "PersistedComponentTypeResolver.hpp"
#include "Component.hpp"
#include "AutomaticScriptComponentRuntimeDeserializer.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "runtime/array.hpp"
#include "Entity.hpp"
#include "EngineBinaryReader.hpp"
#include "SceneAssetReference.hpp"
#include "system/string_comparer.hpp"
#include "GeneratedRuntimeComponentDeserializerRegistration.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/string_comparer.hpp"

::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault()
{
::RuntimeComponentRegistry *registry = new ::RuntimeComponentRegistry();
RuntimeComponentRegistry::RegisterBuiltInComponentDeserializers(registry);
RegisterGeneratedRuntimeComponentDeserializers(registry);
return registry;}

List<std::string>* RuntimeComponentRegistry::GetBuiltInComponentTypeIds()
{
::RuntimeComponentRegistry *registry = new ::RuntimeComponentRegistry();
RuntimeComponentRegistry::RegisterBuiltInComponentDeserializers(registry);
List<std::string> *builtInComponentTypeIds = new List<std::string>(static_cast<int32_t>(registry->DeserializersByTypeId->get_Count()));
for (const auto& componentTypeId : registry->DeserializersByTypeId->Keys()) {
builtInComponentTypeIds->Add(componentTypeId);
}
return builtInComponentTypeIds;}

::IRuntimeComponentDeserializer* RuntimeComponentRegistry::GetDeserializer(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
throw ([&]() {
auto __ctor_arg_0000012D = "Component type id must be provided.";
auto __ctor_arg_0000012E = "componentTypeId";
return new ArgumentException(__ctor_arg_0000012D, __ctor_arg_0000012E);
})();
    }
::IRuntimeComponentDeserializer* deserializer;
    if (!this->DeserializersByTypeId->TryGetValue(componentTypeId, deserializer))
    {
const std::string normalizedEngineComponentTypeId = RuntimeComponentRegistry::NormalizeLegacyEngineComponentTypeId(componentTypeId);
    if (!String::Equals(normalizedEngineComponentTypeId, componentTypeId, StringComparison::Ordinal) && this->DeserializersByTypeId->TryGetValue(normalizedEngineComponentTypeId, deserializer))
    {
return deserializer;    }
deserializer = this->TryCreateAutomaticComponentDeserializer(componentTypeId);
    if (deserializer == nullptr)
    {
throw new InvalidOperationException(std::string("Player builds do not support serialized component type '") + componentTypeId + std::string("' yet."));
    }
this->DeserializersByTypeId->Add(componentTypeId, deserializer);
    }
return deserializer;}

void RuntimeComponentRegistry::Register(::IRuntimeComponentDeserializer* deserializer)
{
    if (deserializer == nullptr)
    {
throw new ArgumentNullException("deserializer");
    }
    if (String::IsNullOrWhiteSpace(deserializer->get_ComponentTypeId()))
    {
throw new InvalidOperationException("Runtime component deserializers must expose a serialized type id.");
    }
    if (this->DeserializersByTypeId->ContainsKey(deserializer->get_ComponentTypeId()))
    {
throw new InvalidOperationException(std::string("A runtime component deserializer is already registered for '") + deserializer->get_ComponentTypeId() + std::string("'."));
    }
this->DeserializersByTypeId->Add(deserializer->get_ComponentTypeId(), deserializer);
}

RuntimeComponentRegistry::RuntimeComponentRegistry() : DeserializersByTypeId()
{
this->DeserializersByTypeId = new Dictionary<std::string, ::IRuntimeComponentDeserializer*>(StringComparer::get_OrdinalIgnoreCase());
}

bool RuntimeComponentRegistry::TryGet__out1(std::string componentTypeId, ::IRuntimeComponentDeserializer*& deserializer)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
deserializer = nullptr;
return false;    }
return this->DeserializersByTypeId->TryGetValue(componentTypeId, deserializer);}

std::string RuntimeComponentRegistry::NormalizeLegacyEngineComponentTypeId(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
return componentTypeId;    }
{
const std::string __switchValue13_1 = componentTypeId;
if (String::Equals(__switchValue13_1, "helengine.AmbientLightComponent, helengine.core")) {
return "helengine.AmbientLightComponent";}
else if (String::Equals(__switchValue13_1, "helengine.AnchorComponent")) {
return "helengine.LayoutComponent";}
else if (String::Equals(__switchValue13_1, "helengine.AnchorComponent, helengine.core")) {
return "helengine.LayoutComponent";}
else if (String::Equals(__switchValue13_1, "helengine.AnimationPlayerComponent, helengine.core")) {
return "helengine.AnimationPlayerComponent";}
else if (String::Equals(__switchValue13_1, "helengine.CameraComponent, helengine.core")) {
return "helengine.CameraComponent";}
else if (String::Equals(__switchValue13_1, "helengine.ClipRectComponent, helengine.core")) {
return "helengine.ClipRectComponent";}
else if (String::Equals(__switchValue13_1, "helengine.DebugComponent, helengine.core")) {
return "helengine.DebugComponent";}
else if (String::Equals(__switchValue13_1, "helengine.DirectionalLightComponent, helengine.core")) {
return "helengine.DirectionalLightComponent";}
else if (String::Equals(__switchValue13_1, "helengine.FPSComponent, helengine.core")) {
return "helengine.FPSComponent";}
else if (String::Equals(__switchValue13_1, "helengine.InteractableComponent, helengine.core")) {
return "helengine.InteractableComponent";}
else if (String::Equals(__switchValue13_1, "helengine.LineRendererComponent, helengine.core")) {
return "helengine.LineRendererComponent";}
else if (String::Equals(__switchValue13_1, "helengine.MeshComponent, helengine.core")) {
return "helengine.MeshComponent";}
else if (String::Equals(__switchValue13_1, "helengine.PointLightComponent, helengine.core")) {
return "helengine.PointLightComponent";}
else if (String::Equals(__switchValue13_1, "helengine.ReferenceCanvasFitComponent, helengine.core")) {
return "helengine.ReferenceCanvasFitComponent";}
else if (String::Equals(__switchValue13_1, "helengine.RoundedRectComponent, helengine.core")) {
return "helengine.RoundedRectComponent";}
else if (String::Equals(__switchValue13_1, "helengine.SceneMapComponent, helengine.core")) {
return "helengine.SceneMapComponent";}
else if (String::Equals(__switchValue13_1, "helengine.SceneMemoryProbeComponent, helengine.core")) {
return "helengine.SceneMemoryProbeComponent";}
else if (String::Equals(__switchValue13_1, "helengine.ScrollComponent, helengine.core")) {
return "helengine.ScrollComponent";}
else if (String::Equals(__switchValue13_1, "helengine.SpotLightComponent, helengine.core")) {
return "helengine.SpotLightComponent";}
else if (String::Equals(__switchValue13_1, "helengine.SpriteComponent, helengine.core")) {
return "helengine.SpriteComponent";}
else if (String::Equals(__switchValue13_1, "helengine.TextComponent, helengine.core")) {
return "helengine.TextComponent";}
else if (String::Equals(__switchValue13_1, "helengine.ViewportComponent, helengine.core")) {
return "helengine.ViewportComponent";}
else if (String::Equals(__switchValue13_1, "helengine.BoxCollider3DComponent, helengine.physics3d")) {
return "helengine.BoxCollider3DComponent";}
else if (String::Equals(__switchValue13_1, "helengine.CharacterController3DComponent, helengine.physics3d")) {
return "helengine.CharacterController3DComponent";}
else if (String::Equals(__switchValue13_1, "helengine.KinematicMotion3DComponent, helengine.physics3d")) {
return "helengine.KinematicMotion3DComponent";}
else if (String::Equals(__switchValue13_1, "helengine.RigidBody3DComponent, helengine.physics3d")) {
return "helengine.RigidBody3DComponent";}
else {
return componentTypeId;}
}

}

void RuntimeComponentRegistry::RegisterBuiltInComponentDeserializers(::RuntimeComponentRegistry* registry)
{
    if (registry == nullptr)
    {
throw new ArgumentNullException("registry");
    }
}

::IRuntimeComponentDeserializer* RuntimeComponentRegistry::TryCreateAutomaticComponentDeserializer(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
return nullptr;    }
Type *componentType = PersistedComponentTypeResolver::TryResolve(componentTypeId);
    if (componentType == nullptr)
    {
return nullptr;    }
    if (!he_cpp_type_of<Component>("Component")->IsAssignableFrom(componentType))
    {
return nullptr;    }
return new ::AutomaticScriptComponentRuntimeDeserializer(componentTypeId, componentType);}

