#ifdef DrawText
#undef DrawText
#endif
#include "ObjectManager.hpp"
#include "runtime/native_list.hpp"
#include "IRenderQueue3D.hpp"
#include "ICamera.hpp"
#include "IRenderQueue2D.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "CameraComponent.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "ObjectManager.hpp"
#include "IUpdateable.hpp"
#include "PendingUpdateOperation.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "IInteractable2D.hpp"
#include "ViewportComponent.hpp"
#include "CoreInitializationOptions.hpp"
#include "IRenderVisitor3D.hpp"
#include "float4.hpp"
#include "RenderTarget.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "IRenderVisitor2D.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "RenderList2D.hpp"
#include "RenderList3D.hpp"
#include "float3.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeModel.hpp"
#include "runtime/array.hpp"
#include "RuntimeMaterial.hpp"
#include "PointerCursorKind.hpp"
#include "PointerInteraction.hpp"
#include "AnchorSpace.hpp"
#include "ViewportLayoutSnapshot.hpp"
#include "float2.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"

List<::Entity*>* ObjectManager::get_Entities()
{
return this->Entities;
}

void ObjectManager::set_Entities(List<::Entity*>* value)
{
this->Entities = value;
}

int32_t ObjectManager::get_EntityCapacity()
{
return this->Entities->get_Capacity();
}

List<::IUpdateable*>* ObjectManager::get_Updateables()
{
return this->Updateables;
}

void ObjectManager::set_Updateables(List<::IUpdateable*>* value)
{
this->Updateables = value;
}

int32_t ObjectManager::get_UpdateableCapacity()
{
return this->Updateables->get_Capacity();
}

bool ObjectManager::get_IsUpdateLoopActive()
{
return this->updateLoopActive;
}

uint8_t ObjectManager::get_UpdateOrderLayers()
{
return this->UpdateOrderLayers;
}

void ObjectManager::set_UpdateOrderLayers(uint8_t value)
{
this->UpdateOrderLayers = value;
}

List<::IDrawable2D*>* ObjectManager::get_Drawables2D()
{
return this->Drawables2D;
}

void ObjectManager::set_Drawables2D(List<::IDrawable2D*>* value)
{
this->Drawables2D = value;
}

int32_t ObjectManager::get_Drawable2DCapacity()
{
return this->Drawables2D->get_Capacity();
}

List<::IDrawable3D*>* ObjectManager::get_Drawables3D()
{
return this->Drawables3D;
}

void ObjectManager::set_Drawables3D(List<::IDrawable3D*>* value)
{
this->Drawables3D = value;
}

int32_t ObjectManager::get_Drawable3DCapacity()
{
return this->Drawables3D->get_Capacity();
}

uint8_t ObjectManager::get_RenderOrderLayers3D()
{
return this->RenderOrderLayers3D;
}

void ObjectManager::set_RenderOrderLayers3D(uint8_t value)
{
this->RenderOrderLayers3D = value;
}

List<::ICamera*>* ObjectManager::get_Cameras()
{
return this->Cameras;
}

void ObjectManager::set_Cameras(List<::ICamera*>* value)
{
this->Cameras = value;
}

List<::DirectionalLightComponent*>* ObjectManager::get_DirectionalLights()
{
return this->DirectionalLights;
}

void ObjectManager::set_DirectionalLights(List<::DirectionalLightComponent*>* value)
{
this->DirectionalLights = value;
}

List<::AmbientLightComponent*>* ObjectManager::get_AmbientLights()
{
return this->AmbientLights;
}

void ObjectManager::set_AmbientLights(List<::AmbientLightComponent*>* value)
{
this->AmbientLights = value;
}

List<::PointLightComponent*>* ObjectManager::get_PointLights()
{
return this->PointLights;
}

void ObjectManager::set_PointLights(List<::PointLightComponent*>* value)
{
this->PointLights = value;
}

List<::SpotLightComponent*>* ObjectManager::get_SpotLights()
{
return this->SpotLights;
}

void ObjectManager::set_SpotLights(List<::SpotLightComponent*>* value)
{
this->SpotLights = value;
}

int32_t ObjectManager::get_CameraCapacity()
{
return this->Cameras->get_Capacity();
}

List<::IInteractable2D*>* ObjectManager::get_Interactables()
{
return this->Interactables;
}

void ObjectManager::set_Interactables(List<::IInteractable2D*>* value)
{
this->Interactables = value;
}

int32_t ObjectManager::get_InteractableCapacity()
{
return this->Interactables->get_Capacity();
}

int32_t ObjectManager::get_PendingUpdateOperationCount()
{
return this->pendingUpdateOperations->get_Count();
}

int32_t ObjectManager::get_PendingUpdateOperationCapacity()
{
return this->pendingUpdateOperations->get_Capacity();
}

uint8_t ObjectManager::GetRenderOrderForLayer3D(int32_t layerIndex)
{
return ObjectManager::GetOrderForLayer(static_cast<int32_t>(layerIndex), static_cast<int32_t>(this->RenderOrderLayers3D));}

uint8_t ObjectManager::GetUpdateOrderForLayer(int32_t layerIndex)
{
return ObjectManager::GetOrderForLayer(static_cast<int32_t>(layerIndex), static_cast<int32_t>(this->UpdateOrderLayers));}

ObjectManager::ObjectManager(::CoreInitializationOptions* settings) : Entities(), Updateables(), UpdateOrderLayers(4), Drawables2D(), Drawables3D(), RenderOrderLayers3D(4), Cameras(), DirectionalLights(), AmbientLights(), PointLights(), SpotLights(), Interactables(), updateLoopActive(), pendingUpdateOperations()
{
    if (settings == nullptr)
    {
throw new ArgumentNullException("settings");
    }
settings->Normalize();
this->set_UpdateOrderLayers(settings->UpdateOrderLayers);
this->set_RenderOrderLayers3D(settings->RenderOrderLayers3D);
this->set_Entities(new List<::Entity*>());
this->set_Updateables(new List<::IUpdateable*>(static_cast<int32_t>(settings->UpdateListInitialCapacity)));
this->pendingUpdateOperations = new List<::PendingUpdateOperation>();
this->set_Drawables2D(new List<::IDrawable2D*>(static_cast<int32_t>(settings->RenderList2DInitialCapacity)));
this->set_Drawables3D(new List<::IDrawable3D*>(static_cast<int32_t>(settings->RenderList3DInitialCapacity)));
this->set_Cameras(new List<::ICamera*>());
this->set_DirectionalLights(new List<::DirectionalLightComponent*>());
this->set_AmbientLights(new List<::AmbientLightComponent*>());
this->set_PointLights(new List<::PointLightComponent*>());
this->set_SpotLights(new List<::SpotLightComponent*>());
this->set_Interactables(new List<::IInteractable2D*>());
}

void ObjectManager::RegisterAmbientLight(::AmbientLightComponent* light)
{
    if (ObjectManager::ContainsReference<AmbientLightComponent*>(this->AmbientLights, light))
    {
return;    }
this->AmbientLights->Add(light);
}

void ObjectManager::RegisterCamera(::ICamera* camera)
{
    if (ObjectManager::ContainsReference<ICamera*>(this->Cameras, camera))
    {
return;    }
this->InsertCameraByDrawOrder(camera);
::IRenderQueue3D *list3D = camera->get_RenderQueue3D();
for (int32_t i = 0; i < this->Drawables3D->get_Count(); i++) {
::IDrawable3D *drawable = (*this->Drawables3D).get_Item(static_cast<int32_t>(i));
    if (this->ShouldRegisterDrawableWithCamera(drawable->get_Parent(), camera))
    {
list3D->Add(drawable);
    }
}
::IRenderQueue2D *list2D = camera->get_RenderQueue2D();
for (int32_t i = 0; i < this->Drawables2D->get_Count(); i++) {
::IDrawable2D *drawable2D = (*this->Drawables2D).get_Item(static_cast<int32_t>(i));
    if (this->ShouldRegisterDrawableWithCamera(drawable2D->get_Parent(), camera))
    {
list2D->Add(drawable2D);
    }
}
}

void ObjectManager::RegisterDirectionalLight(::DirectionalLightComponent* light)
{
    if (ObjectManager::ContainsReference<DirectionalLightComponent*>(this->DirectionalLights, light))
    {
return;    }
this->DirectionalLights->Add(light);
}

void ObjectManager::RegisterEntity(::Entity* entity)
{
    if (ObjectManager::ContainsReference<Entity*>(this->Entities, entity))
    {
return;    }
this->Entities->Add(entity);
}

void ObjectManager::RegisterForRender2D(::IDrawable2D* drawable)
{
    if (ObjectManager::ContainsReference<IDrawable2D*>(this->Drawables2D, drawable))
    {
return;    }
this->Drawables2D->Add(drawable);
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
::ICamera *camera = (*this->Cameras).get_Item(static_cast<int32_t>(i));
    if (!this->ShouldRegisterDrawableWithCamera(drawable->get_Parent(), camera))
    {
continue;
    }
::IRenderQueue2D *list = camera->get_RenderQueue2D();
list->Add(drawable);
}
}

void ObjectManager::RegisterForRender3D(::IDrawable3D* drawable)
{
    if (ObjectManager::ContainsReference<IDrawable3D*>(this->Drawables3D, drawable))
    {
return;    }
this->Drawables3D->Add(drawable);
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
::ICamera *camera = (*this->Cameras).get_Item(static_cast<int32_t>(i));
    if (!this->ShouldRegisterDrawableWithCamera(drawable->get_Parent(), camera))
    {
continue;
    }
::IRenderQueue3D *list = camera->get_RenderQueue3D();
list->Add(drawable);
}
}

void ObjectManager::RegisterForUpdate(::IUpdateable* entity)
{
    if (entity == nullptr)
    {
return;    }
    if (this->updateLoopActive)
    {
this->QueueUpdateOperation(entity, true);
return;    }
this->AddUpdateableToList(entity);
}

void ObjectManager::RegisterInteractable(::IInteractable2D* entity)
{
    if (ObjectManager::ContainsReference<IInteractable2D*>(this->Interactables, entity))
    {
return;    }
this->Interactables->Add(entity);
}

void ObjectManager::RegisterPointLight(::PointLightComponent* light)
{
    if (ObjectManager::ContainsReference<PointLightComponent*>(this->PointLights, light))
    {
return;    }
this->PointLights->Add(light);
}

void ObjectManager::RegisterSpotLight(::SpotLightComponent* light)
{
    if (ObjectManager::ContainsReference<SpotLightComponent*>(this->SpotLights, light))
    {
return;    }
this->SpotLights->Add(light);
}

void ObjectManager::RemoveAmbientLight(::AmbientLightComponent* light)
{
ObjectManager::RemoveByReference<AmbientLightComponent*>(this->AmbientLights, light);
}

void ObjectManager::RemoveCamera(::ICamera* camera)
{
    if (camera == nullptr)
    {
return;    }
ObjectManager::RemoveByReference<ICamera*>(this->Cameras, camera);
camera->get_RenderQueue3D()->Clear();
camera->get_RenderQueue2D()->Clear();
}

void ObjectManager::RemoveDirectionalLight(::DirectionalLightComponent* light)
{
ObjectManager::RemoveByReference<DirectionalLightComponent*>(this->DirectionalLights, light);
}

void ObjectManager::RemoveEntity(::Entity* entity)
{
ObjectManager::RemoveByReference<Entity*>(this->Entities, entity);
}

void ObjectManager::RemoveFromRender2D(::IDrawable2D* drawable)
{
ObjectManager::RemoveByReference<IDrawable2D*>(this->Drawables2D, drawable);
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
::ICamera *camera = (*this->Cameras).get_Item(static_cast<int32_t>(i));
::IRenderQueue2D *list = camera->get_RenderQueue2D();
list->Remove(drawable);
}
}

void ObjectManager::RemoveFromRender3D(::IDrawable3D* drawable)
{
ObjectManager::RemoveByReference<IDrawable3D*>(this->Drawables3D, drawable);
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
::ICamera *camera = (*this->Cameras).get_Item(static_cast<int32_t>(i));
::IRenderQueue3D *list = camera->get_RenderQueue3D();
list->Remove(drawable);
}
}

void ObjectManager::RemoveFromUpdate(::IUpdateable* entity, uint8_t lastUpdateOrder)
{
    if (entity == nullptr)
    {
return;    }
    if (this->updateLoopActive)
    {
this->QueueUpdateOperation(entity, false);
return;    }
this->RemoveUpdateableFromList(entity);
}

void ObjectManager::RemoveInteractable(::IInteractable2D* entity)
{
ObjectManager::RemoveByReference<IInteractable2D*>(this->Interactables, entity);
}

void ObjectManager::RemovePointLight(::PointLightComponent* light)
{
ObjectManager::RemoveByReference<PointLightComponent*>(this->PointLights, light);
}

void ObjectManager::RemoveSpotLight(::SpotLightComponent* light)
{
ObjectManager::RemoveByReference<SpotLightComponent*>(this->SpotLights, light);
}

void ObjectManager::ReserveRender2DCapacity(uint8_t renderOrder, int32_t additional, uint16_t layerMask)
{
    if (additional < 1)
    {
return;    }
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
::ICamera *camera = (*this->Cameras).get_Item(static_cast<int32_t>(i));
    if ((layerMask & camera->get_LayerMask()) == 0)
    {
continue;
    }
::IRenderQueue2D *list = camera->get_RenderQueue2D();
list->EnsureCapacity(static_cast<int32_t>(list->get_Count() + additional));
}
}

void ObjectManager::ReserveRender3DCapacity(uint8_t renderOrder, int32_t additional, uint16_t layerMask)
{
    if (additional < 1)
    {
return;    }
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
::ICamera *camera = (*this->Cameras).get_Item(static_cast<int32_t>(i));
    if ((layerMask & camera->get_LayerMask()) == 0)
    {
continue;
    }
::IRenderQueue3D *list = camera->get_RenderQueue3D();
list->EnsureCapacity(static_cast<int32_t>(list->get_Count() + additional));
}
}

void ObjectManager::ReserveUpdateCapacity(uint8_t updateOrder, int32_t additional)
{
    if (additional < 1)
    {
return;    }
const int32_t desired = this->Updateables->get_Count() + additional;
    if (this->Updateables->get_Capacity() < desired)
    {
this->Updateables->SetCapacity(desired);
    }
}

void ObjectManager::Update()
{
{
auto __finallyGuard_00000100 = he_cpp_make_scope_exit([&]() {
this->updateLoopActive = false;
});
this->updateLoopActive = true;
for (int32_t i = 0; i < this->Updateables->get_Count(); i++) {
::IUpdateable *item = (*this->Updateables).get_Item(static_cast<int32_t>(i));
item->Update();
}
}
this->ApplyPendingUpdateOperations();
}

void ObjectManager::AddUpdateableToList(::IUpdateable* entity)
{
const int32_t insertIndex = this->FindUpdateInsertIndex(static_cast<uint8_t>(entity->get_UpdateOrder()));
this->Updateables->Insert(static_cast<int32_t>(insertIndex), entity);
}

void ObjectManager::ApplyPendingUpdateOperations()
{
    if (this->pendingUpdateOperations->get_Count() == 0)
    {
return;    }
for (int32_t i = 0; i < this->pendingUpdateOperations->get_Count(); i++) {
::PendingUpdateOperation op = (*this->pendingUpdateOperations).get_Item(static_cast<int32_t>(i));
    if (op.IsAdd)
    {
this->AddUpdateableToList(op.Entity);
    }
else {
this->RemoveUpdateableFromList(op.Entity);
}
}
this->pendingUpdateOperations->Clear();
}

template <typename T>
bool ObjectManager::ContainsReference(List<T>* list, T item)
{
for (int32_t index = 0; index < list->get_Count(); index++) {
    if (((*list).get_Item(static_cast<int32_t>(index)) == item))
    {
return true;    }
}
return false;}

int32_t ObjectManager::FindUpdateInsertIndex(uint8_t updateOrder)
{
for (int32_t i = 0; i < this->Updateables->get_Count(); i++) {
    if (updateOrder < (*this->Updateables).get_Item(static_cast<int32_t>(i))->get_UpdateOrder())
    {
return i;    }
}
return this->Updateables->get_Count();}

uint8_t ObjectManager::GetOrderForLayer(int32_t layerIndex, int32_t layerCount)
{
    if (layerCount < 1)
    {
throw new ArgumentOutOfRangeException("layerCount");
    }
int32_t clamped = layerIndex;
    if (clamped < 0)
    {
clamped = 0;
    }
else {
    if (clamped >= layerCount)
    {
clamped = layerCount - 1;
    }
}
    if (layerCount == 1)
    {
return 0;    }
const double step = 255.0 / (layerCount - 1);
const int32_t value = static_cast<int32_t>(Math::Round(clamped * step, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero)));
    if (value < 0)
    {
return 0;    }
    if (value > 255)
    {
return 255;    }
return static_cast<uint8_t>(value);}

void ObjectManager::InsertCameraByDrawOrder(::ICamera* camera)
{
int32_t insertIndex = this->Cameras->get_Count();
const uint8_t order = camera->get_CameraDrawOrder();
for (int32_t i = 0; i < this->Cameras->get_Count(); i++) {
    if (order < (*this->Cameras).get_Item(static_cast<int32_t>(i))->get_CameraDrawOrder())
    {
insertIndex = i;
break;
    }
}
this->Cameras->Insert(static_cast<int32_t>(insertIndex), camera);
}

void ObjectManager::QueueUpdateOperation(::IUpdateable* entity, bool isAdd)
{
this->pendingUpdateOperations->Add(::PendingUpdateOperation(entity, isAdd));
}

template <typename T>
bool ObjectManager::RemoveByReference(List<T>* list, T item)
{
bool removed = false;
for (int32_t index = list->get_Count() - 1; index >= 0; index--) {
    if (!((*list).get_Item(static_cast<int32_t>(index)) == item))
    {
continue;
    }
list->RemoveAt(static_cast<int32_t>(index));
removed = true;
}
return removed;}

void ObjectManager::RemoveUpdateableFromList(::IUpdateable* entity)
{
ObjectManager::RemoveByReference<IUpdateable*>(this->Updateables, entity);
}

::ICameraBoundViewportOwner* ObjectManager::ResolveNearestViewportComponent(::Entity* entity)
{
::Entity *current = entity;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->get_Count(); componentIndex++) {
    ICameraBoundViewportOwner* viewportComponent = he_cpp_try_cast<ICameraBoundViewportOwner>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (viewportComponent != nullptr)
    {
return viewportComponent;    }
}
    }
current = current->Parent;
}
return nullptr;}

bool ObjectManager::ShouldRegisterDrawableWithCamera(::Entity* drawableOwner, ::ICamera* camera)
{
    if (drawableOwner == nullptr || camera == nullptr)
    {
return false;    }
    if ((drawableOwner->get_LayerMask() & camera->get_LayerMask()) == 0)
    {
return false;    }
::ICameraBoundViewportOwner *viewportComponent = this->ResolveNearestViewportComponent(drawableOwner);
    if (viewportComponent == nullptr)
    {
return true;    }
::CameraComponent *boundCamera = viewportComponent->GetBoundCameraComponent();
    if (viewportComponent->get_BindingMode() == ViewportComponent::ExplicitCameraBindingMode)
    {
return (boundCamera == camera);    }
    if (viewportComponent->get_BindingMode() == ViewportComponent::AncestorCameraBindingMode && boundCamera != nullptr)
    {
return (boundCamera == camera);    }
return true;}

