#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class float4;
class RenderTarget;
class CameraClearSettings;
class CameraRenderSettings;
class IRenderQueue2D;
class IRenderQueue3D;

#include "float4.hpp"
#include "CameraClearSettings.hpp"

class ICamera
{
public:
    virtual ::Entity* get_Parent() = 0;

    virtual uint16_t get_LayerMask() = 0;

    virtual void set_LayerMask(uint16_t value) = 0;

    virtual uint8_t get_CameraDrawOrder() = 0;

    virtual void set_CameraDrawOrder(uint8_t value) = 0;

    virtual ::float4 get_Viewport() = 0;

    virtual void set_Viewport(::float4 value) = 0;

    virtual float get_NearPlaneDistance() = 0;

    virtual void set_NearPlaneDistance(float value) = 0;

    virtual float get_FarPlaneDistance() = 0;

    virtual void set_FarPlaneDistance(float value) = 0;

    virtual ::RenderTarget* get_RenderTarget() = 0;

    virtual void set_RenderTarget(::RenderTarget* value) = 0;

    virtual ::CameraClearSettings get_ClearSettings() = 0;

    virtual void set_ClearSettings(::CameraClearSettings value) = 0;

    virtual ::CameraRenderSettings* get_RenderSettings() = 0;

    virtual void set_RenderSettings(::CameraRenderSettings* value) = 0;

    virtual ::IRenderQueue2D* get_RenderQueue2D() = 0;

    virtual ::IRenderQueue3D* get_RenderQueue3D() = 0;
};
