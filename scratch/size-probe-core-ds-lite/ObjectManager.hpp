#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class IUpdateable;
class IDrawable2D;
class IDrawable3D;
class ICamera;
class DirectionalLightComponent;
class AmbientLightComponent;
class PointLightComponent;
class SpotLightComponent;
class IInteractable2D;
class PendingUpdateOperation;
class CoreInitializationOptions;
class ICameraBoundViewportOwner;

#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "PendingUpdateOperation.hpp"
#include "runtime/native_list.hpp"

class ObjectManager
{
public:
    virtual ~ObjectManager() = default;

    List<::Entity*>* Entities;

    List<::Entity*>* get_Entities();
    void set_Entities(List<::Entity*>* value);

    int32_t get_EntityCapacity();

    List<::IUpdateable*>* Updateables;

    List<::IUpdateable*>* get_Updateables();
    void set_Updateables(List<::IUpdateable*>* value);

    int32_t get_UpdateableCapacity();

    bool get_IsUpdateLoopActive();

    uint8_t UpdateOrderLayers;

    uint8_t get_UpdateOrderLayers();
    void set_UpdateOrderLayers(uint8_t value);

    List<::IDrawable2D*>* Drawables2D;

    List<::IDrawable2D*>* get_Drawables2D();
    void set_Drawables2D(List<::IDrawable2D*>* value);

    int32_t get_Drawable2DCapacity();

    List<::IDrawable3D*>* Drawables3D;

    List<::IDrawable3D*>* get_Drawables3D();
    void set_Drawables3D(List<::IDrawable3D*>* value);

    int32_t get_Drawable3DCapacity();

    uint8_t RenderOrderLayers3D;

    uint8_t get_RenderOrderLayers3D();
    void set_RenderOrderLayers3D(uint8_t value);

    List<::ICamera*>* Cameras;

    List<::ICamera*>* get_Cameras();
    void set_Cameras(List<::ICamera*>* value);

    List<::DirectionalLightComponent*>* DirectionalLights;

    List<::DirectionalLightComponent*>* get_DirectionalLights();
    void set_DirectionalLights(List<::DirectionalLightComponent*>* value);

    List<::AmbientLightComponent*>* AmbientLights;

    List<::AmbientLightComponent*>* get_AmbientLights();
    void set_AmbientLights(List<::AmbientLightComponent*>* value);

    List<::PointLightComponent*>* PointLights;

    List<::PointLightComponent*>* get_PointLights();
    void set_PointLights(List<::PointLightComponent*>* value);

    List<::SpotLightComponent*>* SpotLights;

    List<::SpotLightComponent*>* get_SpotLights();
    void set_SpotLights(List<::SpotLightComponent*>* value);

    int32_t get_CameraCapacity();

    List<::IInteractable2D*>* Interactables;

    List<::IInteractable2D*>* get_Interactables();
    void set_Interactables(List<::IInteractable2D*>* value);

    int32_t get_InteractableCapacity();

    int32_t get_PendingUpdateOperationCount();

    int32_t get_PendingUpdateOperationCapacity();

    uint8_t GetRenderOrderForLayer3D(int32_t layerIndex);

    uint8_t GetUpdateOrderForLayer(int32_t layerIndex);

    ObjectManager(::CoreInitializationOptions* settings);

    void RegisterAmbientLight(::AmbientLightComponent* light);

    void RegisterCamera(::ICamera* camera);

    void RegisterDirectionalLight(::DirectionalLightComponent* light);

    virtual void RegisterEntity(::Entity* entity);

    void RegisterForRender2D(::IDrawable2D* drawable);

    void RegisterForRender3D(::IDrawable3D* drawable);

    virtual void RegisterForUpdate(::IUpdateable* entity);

    virtual void RegisterInteractable(::IInteractable2D* entity);

    void RegisterPointLight(::PointLightComponent* light);

    void RegisterSpotLight(::SpotLightComponent* light);

    void RemoveAmbientLight(::AmbientLightComponent* light);

    virtual void RemoveCamera(::ICamera* camera);

    void RemoveDirectionalLight(::DirectionalLightComponent* light);

    virtual void RemoveEntity(::Entity* entity);

    void RemoveFromRender2D(::IDrawable2D* drawable);

    void RemoveFromRender3D(::IDrawable3D* drawable);

    virtual void RemoveFromUpdate(::IUpdateable* entity, uint8_t lastUpdateOrder);

    virtual void RemoveInteractable(::IInteractable2D* entity);

    void RemovePointLight(::PointLightComponent* light);

    void RemoveSpotLight(::SpotLightComponent* light);

    void ReserveRender2DCapacity(uint8_t renderOrder, int32_t additional, uint16_t layerMask);

    void ReserveRender3DCapacity(uint8_t renderOrder, int32_t additional, uint16_t layerMask);

    void ReserveUpdateCapacity(uint8_t updateOrder, int32_t additional);

    virtual void Update();
private:
    bool updateLoopActive;

    List<::PendingUpdateOperation>* pendingUpdateOperations;

    void AddUpdateableToList(::IUpdateable* entity);

    void ApplyPendingUpdateOperations();

    template <typename T>
    static bool ContainsReference(List<T>* list, T item);

    int32_t FindUpdateInsertIndex(uint8_t updateOrder);

    static uint8_t GetOrderForLayer(int32_t layerIndex, int32_t layerCount);

    void InsertCameraByDrawOrder(::ICamera* camera);

    void QueueUpdateOperation(::IUpdateable* entity, bool isAdd);

    template <typename T>
    static bool RemoveByReference(List<T>* list, T item);

    void RemoveUpdateableFromList(::IUpdateable* entity);

    ::ICameraBoundViewportOwner* ResolveNearestViewportComponent(::Entity* entity);

    bool ShouldRegisterDrawableWithCamera(::Entity* drawableOwner, ::ICamera* camera);
};
