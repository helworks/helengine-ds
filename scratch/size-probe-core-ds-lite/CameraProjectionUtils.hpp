#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4x4;
class ICamera;

#include "float4x4.hpp"

class CameraProjectionUtils
{
public:
    virtual ~CameraProjectionUtils() = default;

    inline static const float MinimumNearPlaneDistance = 0.01f;

    inline static const float MinimumPlaneSeparation = 0.01f;

    static float ClampFarPlaneDistance(float nearPlaneDistance, float farPlaneDistance);

    static float ClampNearPlaneDistance(float nearPlaneDistance, float farPlaneDistance);

    static ::float4x4 CreatePerspectiveProjection(::ICamera* camera, float fieldOfView, float aspectRatio);
};
