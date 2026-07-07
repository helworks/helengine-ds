#ifdef DrawText
#undef DrawText
#endif
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "Entity.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "ComponentExecutionPolicy.hpp"
#include "float4x4.hpp"
#include "NativeOwnership.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "SceneManager.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"
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
#include "runtime/native_event.hpp"
#include "LoadedSceneRecord.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "PendingSceneOperation.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "SceneLoadMode.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "SceneAsset.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "float2.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "system/diagnostics/stopwatch.hpp"

::float3 Entity::get_Position()
{
this->ThrowIfDisposed();
::float3 pos = this->position;
    if (this->Parent != nullptr)
    {
::float3 scaledLocal = pos * this->Parent->get_Scale();
::float3 rotatedLocal = float4::RotateVector(scaledLocal, this->Parent->get_Orientation());
pos = rotatedLocal + this->Parent->get_Position();
    }
return pos;}

void Entity::set_Position(::float3 value)
{
this->ThrowIfDisposed();
this->position = value;
}

::float3 Entity::get_LocalPosition()
{
this->ThrowIfDisposed();
return this->position;}

void Entity::set_LocalPosition(::float3 value)
{
this->ThrowIfDisposed();
this->position = value;
}

::float3 Entity::get_Scale()
{
this->ThrowIfDisposed();
::float3 sca = this->scale;
    if (this->Parent != nullptr)
    {
sca = sca * this->Parent->get_Scale();
    }
return sca;}

void Entity::set_Scale(::float3 value)
{
this->ThrowIfDisposed();
this->scale = value;
}

::float3 Entity::get_LocalScale()
{
this->ThrowIfDisposed();
return this->scale;}

void Entity::set_LocalScale(::float3 value)
{
this->ThrowIfDisposed();
this->scale = value;
}

::float4 Entity::get_Orientation()
{
this->ThrowIfDisposed();
::float4 ori = this->orientation;
    if (this->Parent != nullptr)
    {
::float4 parentOrientation = this->Parent->get_Orientation();
float4::Concatenate__ref0_ref1_out2(ori, parentOrientation, ori);
    }
return ori;}

void Entity::set_Orientation(::float4 value)
{
this->ThrowIfDisposed();
this->orientation = value;
}

::float4 Entity::get_LocalOrientation()
{
this->ThrowIfDisposed();
return this->orientation;}

void Entity::set_LocalOrientation(::float4 value)
{
this->ThrowIfDisposed();
this->orientation = value;
}

::float4x4 Entity::get_LocalTransformMatrix()
{
this->ThrowIfDisposed();
return Entity::CreateTransformMatrix(this->get_LocalPosition(), this->get_LocalScale(), this->get_LocalOrientation());}

::float4x4 Entity::get_WorldTransformMatrix()
{
this->ThrowIfDisposed();
::float4x4 localTransform = this->get_LocalTransformMatrix();
    if (this->Parent == nullptr)
    {
return localTransform;    }
::float4x4 parentWorldTransform = this->Parent->get_WorldTransformMatrix();
::float4x4 worldTransform;
float4x4::Multiply__ref0_ref1_out2(localTransform, parentWorldTransform, worldTransform);
return worldTransform;}

::Entity* Entity::get_Parent()
{
return this->Parent;
}

void Entity::set_Parent(::Entity* value)
{
this->Parent = value;
}

::Entity* Entity::get_ParentUnsafe()
{
return this->Parent;
}

uint16_t Entity::get_LayerMask()
{
this->ThrowIfDisposed();
return this->layerMask;}

void Entity::set_LayerMask(uint16_t value)
{
this->ThrowIfDisposed();
this->layerMask = value;
}

List<::Component*>* Entity::get_Components()
{
this->ThrowIfDisposed();
return this->components;}

void Entity::set_Components(List<::Component*>* value)
{
this->components = value;
}

List<::Entity*>* Entity::get_Children()
{
this->ThrowIfDisposed();
return this->children;}

void Entity::set_Children(List<::Entity*>* value)
{
this->children = value;
}

bool Entity::get_Enabled()
{
this->ThrowIfDisposed();
return this->isEnabled;}

void Entity::set_Enabled(bool value)
{
this->ThrowIfDisposed();
const bool wasHierarchyEnabled = this->get_IsHierarchyEnabled();
    if (this->isEnabled != value)
    {
this->isEnabled = value;
const bool isHierarchyEnabled = this->get_IsHierarchyEnabled();
    if (wasHierarchyEnabled != isHierarchyEnabled)
    {
this->ParentEnabledChange(isHierarchyEnabled);
    }
    }
}

bool Entity::get_IsHierarchyEnabled()
{
this->ThrowIfDisposed();
    if (!this->isEnabled)
    {
return false;    }
    if (this->Parent == nullptr)
    {
return true;    }
return this->Parent->get_IsHierarchyEnabled();}

bool Entity::get_IsInitialized()
{
this->ThrowIfDisposed();
return this->isInitialized;}

bool Entity::get_IsDisposed()
{
return this->isDisposed;
}

bool Entity::get_Static()
{
this->ThrowIfDisposed();
return this->isStatic;}

void Entity::set_Static(bool value)
{
this->ThrowIfDisposed();
    if (this->isStatic != value)
    {
this->ParentStaticChange(value);
    }
this->isStatic = value;
}

void Entity::AddChild(::Entity* entity)
{
this->ThrowIfDisposed();
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
entity->ThrowIfDisposed();
    if (this->children == nullptr)
    {
throw new InvalidOperationException("Children collection has not been initialized.");
    }
    if (entity->Parent != nullptr)
    {
throw new Exception("Parent is not empty");
    }
const bool wasHierarchyEnabled = entity->get_IsHierarchyEnabled();
entity->set_Parent(this);
this->children->Add(entity);
    if (this->isInitialized)
    {
entity->InitializeHierarchy();
    }
    if (wasHierarchyEnabled && entity->get_IsHierarchyEnabled())
    {
entity->RefreshRegistrationsAfterParentChange();
    }
const bool isHierarchyEnabled = entity->get_IsHierarchyEnabled();
    if (wasHierarchyEnabled != isHierarchyEnabled)
    {
entity->ParentEnabledChange(isHierarchyEnabled);
    }
}

void Entity::AddComponent(::Component* comp)
{
this->ThrowIfDisposed();
    if (comp == nullptr)
    {
throw new ArgumentNullException("comp");
    }
comp->ThrowIfDisposed();
    if (this->components == nullptr)
    {
throw new InvalidOperationException("Components collection has not been initialized.");
    }
    if (comp->get_ParentUnsafe() != nullptr)
    {
throw new InvalidOperationException("Component is already attached to an entity.");
    }
this->components->Add(comp);
comp->AttachToEntity(this);
    if (ComponentExecutionPolicy::ShouldRunComponentLifecycle(comp, this))
    {
comp->ComponentAdded(this);
    if (this->isInitialized)
    {
comp->ComponentInitialized(this);
    }
    }
}

void Entity::Dispose()
{
    if (this->isDisposing)
    {
return;    }
this->isDisposing = true;
List<::Component*> *detachedComponents = nullptr;
    if (this->components != nullptr)
    {
detachedComponents = new List<::Component*>(static_cast<int32_t>(this->components->get_Count()));
while (this->components->get_Count() > 0) {
const int32_t componentIndex = this->components->get_Count() - 1;
this->ReportDisposalStage("BeforeComponentRemove", static_cast<int32_t>(componentIndex));
::Component *component = (*this->components).get_Item(static_cast<int32_t>(this->components->get_Count() - 1));
this->RemoveComponent(component);
detachedComponents->Add(component);
}
List<::Component*> *disposedComponents = this->components;
this->ReportDisposalStage("BeforeComponentsListDelete", static_cast<int32_t>(-1));
this->components = nullptr;
delete disposedComponents;
    }
    if (this->children != nullptr)
    {
while (this->children->get_Count() > 0) {
this->ReportDisposalStage("BeforeChildRemove", static_cast<int32_t>(-1));
::Entity *child = (*this->children).get_Item(static_cast<int32_t>(this->children->get_Count() - 1));
this->RemoveChild(child);
this->ReportChildDisposalStage("BeforeChildDispose", child);
if (child != nullptr)
{
child->Dispose();
delete child;
}
this->ReportDisposalStage("AfterChildDispose", static_cast<int32_t>(-1));
}
List<::Entity*> *disposedChildren = this->children;
this->ReportDisposalStage("BeforeChildrenListDelete", static_cast<int32_t>(-1));
this->children = nullptr;
delete disposedChildren;
    }
    if (detachedComponents != nullptr)
    {
for (int32_t i = 0; i < detachedComponents->get_Count(); i++) {
::Component *component = (*detachedComponents).get_Item(static_cast<int32_t>(i));
this->ReportDisposalStage("BeforeComponentDispose", static_cast<int32_t>(i));
component->Dispose();
this->ReportDisposalStage("AfterComponentDispose", static_cast<int32_t>(i));
delete component;
this->ReportDisposalStage("AfterComponentDelete", static_cast<int32_t>(i));
}
    }
    if (this->Parent != nullptr)
    {
this->ReportDisposalStage("BeforeParentDetach", static_cast<int32_t>(-1));
this->Parent->RemoveChild(this);
    }
this->ReportDisposalStage("BeforeObjectManagerRemoveEntity", static_cast<int32_t>(-1));
Core::Instance->ObjectManager->RemoveEntity(this);
this->ReportDisposalStage("AfterObjectManagerRemoveEntity", static_cast<int32_t>(-1));
this->isDisposed = true;
}

Entity::Entity() : Parent(), isEnabled(), isStatic(), isInitialized(), isDisposing(), isDisposed(), position(), scale(), orientation(), layerMask(), components(), children()
{
this->isEnabled = true;
this->set_Orientation(float4::get_Identity());
this->set_Scale(float3::get_One());
this->set_LayerMask(0b00000001);
Core::Instance->ObjectManager->RegisterEntity(this);
}

void Entity::InitChildren()
{
this->ThrowIfDisposed();
this->children = new List<::Entity*>();
}

void Entity::InitComponents()
{
this->ThrowIfDisposed();
this->components = new List<::Component*>();
}

void Entity::InitializeHierarchy()
{
this->ThrowIfDisposed();
    if (this->get_IsInitialized())
    {
return;    }
this->isInitialized = true;
    if (this->components != nullptr)
    {
for (int32_t i = 0; i < this->components->get_Count(); i++) {
::Component *component = (*this->components).get_Item(static_cast<int32_t>(i));
    if (!ComponentExecutionPolicy::ShouldRunComponentLifecycle(component, this))
    {
continue;
    }
component->ComponentInitialized(this);
}
    }
    if (this->children != nullptr)
    {
for (int32_t i = 0; i < this->children->get_Count(); i++) {
(*this->children).get_Item(static_cast<int32_t>(i))->InitializeHierarchy();
}
    }
}

void Entity::RemoveChild(::Entity* entity)
{
this->ThrowIfDisposed();
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
else {
    if (this->children == nullptr)
    {
throw new InvalidOperationException("Children collection has not been initialized.");
    }
else {
    if (entity->Parent != this)
    {
throw new InvalidOperationException("Entity is not parented to this parent.");
    }
}
}
const bool wasHierarchyEnabled = entity->get_IsHierarchyEnabled();
    if (!this->children->Remove(entity))
    {
throw new InvalidOperationException("Entity could not be removed from the child collection.");
    }
entity->set_Parent(nullptr);
    if (!this->ShouldSuppressRegistrationRefreshForDetachment(entity) && wasHierarchyEnabled && entity->get_IsHierarchyEnabled())
    {
entity->RefreshRegistrationsAfterParentChange();
    }
const bool isHierarchyEnabled = entity->get_IsHierarchyEnabled();
    if (wasHierarchyEnabled != isHierarchyEnabled)
    {
entity->ParentEnabledChange(isHierarchyEnabled);
    }
}

void Entity::RemoveComponent(::Component* comp)
{
this->ThrowIfDisposed();
    if (comp == nullptr)
    {
throw new ArgumentNullException("comp");
    }
else {
    if (this->components == nullptr)
    {
throw new InvalidOperationException("Components collection has not been initialized.");
    }
else {
    if (comp->get_ParentUnsafe() != this)
    {
throw new InvalidOperationException("Component is not attached to this entity.");
    }
}
}
    if (!this->components->Remove(comp))
    {
throw new InvalidOperationException("Component could not be removed from the component collection.");
    }
const bool shouldRunLifecycle = ComponentExecutionPolicy::ShouldRunComponentLifecycle(comp, this);
    if (this->get_IsHierarchyEnabled() && shouldRunLifecycle)
    {
comp->ParentEnabledChange(false);
    }
    if (shouldRunLifecycle)
    {
comp->ComponentRemoved(this);
    }
comp->DetachFromEntity();
}

void Entity::ParentEnabledChange(bool newEnabled)
{
    if (this->components != nullptr)
    {
for (int32_t i = 0; i < this->components->get_Count(); i++) {
::Component *component = (*this->components).get_Item(static_cast<int32_t>(i));
    if (!ComponentExecutionPolicy::ShouldRunComponentLifecycle(component, this))
    {
continue;
    }
component->ParentEnabledChange(newEnabled);
}
    }
    if (this->children != nullptr)
    {
for (int32_t i = 0; i < this->children->get_Count(); i++) {
(*this->children).get_Item(static_cast<int32_t>(i))->ParentEnabledChange((*this->children).get_Item(static_cast<int32_t>(i))->get_IsHierarchyEnabled());
}
    }
}

void Entity::ParentStaticChange(bool newEnabled)
{
    if (this->components != nullptr)
    {
for (int32_t i = 0; i < this->components->get_Count(); i++) {
(*this->components).get_Item(static_cast<int32_t>(i))->ParentStaticChange(newEnabled);
}
    }
    if (this->children != nullptr)
    {
for (int32_t i = 0; i < this->children->get_Count(); i++) {
(*this->children).get_Item(static_cast<int32_t>(i))->ParentStaticChange(newEnabled);
}
    }
}

::float4x4 Entity::CreateTransformMatrix(::float3 position, ::float3 scale, ::float4 orientation)
{
::float4x4 rotation;
float4x4::CreateFromQuaternion__ref0_out1(orientation, rotation);
::float4x4 size;
float4x4::CreateScale__out3(scale.X, scale.Y, scale.Z, size);
::float4x4 scaleRotation;
float4x4::Multiply__ref0_ref1_out2(size, rotation, scaleRotation);
::float4x4 translation;
float4x4::CreateTranslation__ref0_out1(position, translation);
::float4x4 transform;
float4x4::Multiply__ref0_ref1_out2(scaleRotation, translation, transform);
return transform;}

void Entity::RefreshRegistrationsAfterParentChange()
{
    if (!this->get_IsHierarchyEnabled() || Core::Instance == nullptr || Core::Instance->ObjectManager == nullptr)
    {
return;    }
Entity::RefreshRegistrationsAfterParentChangeRecursive(this);
}

void Entity::RefreshRegistrationsAfterParentChangeRecursive(::Entity* entity)
{
    if (entity->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
::Component *component = (*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex));
    IDrawable2D* drawable2D = he_cpp_try_cast<IDrawable2D>(component);
    if (drawable2D != nullptr)
    {
Core::Instance->ObjectManager->RemoveFromRender2D(drawable2D);
Core::Instance->ObjectManager->RegisterForRender2D(drawable2D);
    }
else {
    IDrawable3D* drawable3D = he_cpp_try_cast<IDrawable3D>(component);
    if (drawable3D != nullptr)
    {
Core::Instance->ObjectManager->RemoveFromRender3D(drawable3D);
Core::Instance->ObjectManager->RegisterForRender3D(drawable3D);
    }
else {
    ICamera* camera = he_cpp_try_cast<ICamera>(component);
    if (camera != nullptr)
    {
Core::Instance->ObjectManager->RemoveCamera(camera);
Core::Instance->ObjectManager->RegisterCamera(camera);
    }
}
}
}
    }
    if (entity->get_Children() == nullptr)
    {
return;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->get_Count(); childIndex++) {
Entity::RefreshRegistrationsAfterParentChangeRecursive((*entity->get_Children()).get_Item(static_cast<int32_t>(childIndex)));
}
}

void Entity::ReportChildDisposalStage(std::string stage, ::Entity* entity, int32_t componentIndex)
{
    if (Core::Instance == nullptr)
    {
return;    }
::SceneManager *sceneManager = Core::Instance->SceneManager;
    if (sceneManager == nullptr)
    {
return;    }
sceneManager->ReportEntityDisposalStage(stage, entity, static_cast<int32_t>(componentIndex));
}

void Entity::ReportChildDisposalStage(std::string stage, ::Entity* entity)
{
this->ReportChildDisposalStage(stage, entity, static_cast<int32_t>(-1));
}

void Entity::ReportDisposalStage(std::string stage, int32_t componentIndex)
{
this->ReportChildDisposalStage(stage, this, static_cast<int32_t>(componentIndex));
}

bool Entity::ShouldSuppressRegistrationRefreshForDetachment(::Entity* entity)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
return this->isDisposing || entity->isDisposing;}

void Entity::ThrowIfDisposed()
{
    if (this->isDisposed)
    {
throw new InvalidOperationException("Disposed entities cannot be used.");
    }
}

