#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;
class int2;
class CameraComponent;

#include "float4.hpp"
#include "int2.hpp"

class ICameraBoundViewportOwner
{
public:
    virtual uint8_t get_BindingMode() = 0;

    virtual ::float4 get_ResolvedViewportBounds() = 0;

    virtual ::int2 get_ResolvedViewportSize() = 0;

    virtual ::CameraComponent* GetBoundCameraComponent() = 0;
};
