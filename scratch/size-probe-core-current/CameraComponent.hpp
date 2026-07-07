#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class ICamera;
class float4;
class RenderTarget;
class CameraClearSettings;
class CameraRenderSettings;
class IRenderQueue2D;
class IRenderQueue3D;
class RenderList2D;
class RenderList3D;
class Entity;

#include "Component.hpp"
#include "ICamera.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_event.hpp"
#include "float4.hpp"
#include "CameraClearSettings.hpp"

class CameraComponent : public ::Component, public ::ICamera
{
public:
    virtual ~CameraComponent() = default;

    ::Event ViewportChanged;

    uint8_t get_CameraDrawOrder();

    void set_CameraDrawOrder(uint8_t value);

    ::float4 get_Viewport();

    void set_Viewport(::float4 value);

    float get_NearPlaneDistance();

    void set_NearPlaneDistance(float value);

    float get_FarPlaneDistance();

    void set_FarPlaneDistance(float value);

    ::RenderTarget* RenderTarget;

    ::RenderTarget* get_RenderTarget();
    void set_RenderTarget(::RenderTarget* value);

    ::CameraClearSettings ClearSettings;

    ::CameraClearSettings get_ClearSettings();
    void set_ClearSettings(::CameraClearSettings value);

    ::CameraRenderSettings* get_RenderSettings();

    void set_RenderSettings(::CameraRenderSettings* value);

    ::IRenderQueue2D* get_RenderQueue2D();

    ::IRenderQueue3D* get_RenderQueue3D();

    uint16_t get_LayerMask();

    void set_LayerMask(uint16_t value);

    CameraComponent();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void Dispose();

    void ParentEnabledChange(bool newEnabled);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    uint8_t cameraDrawOrder;

    uint16_t layerMask;

    ::float4 viewportValue;

    ::CameraRenderSettings* RenderSettingsValue;

    float NearPlaneDistanceValue;

    float FarPlaneDistanceValue;

    ::RenderList2D* renderList2D;

    ::RenderList3D* renderList3D;

    void InitializeLists();

    void RaiseViewportChanged();
};
