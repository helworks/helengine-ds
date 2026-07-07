#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;

#include "float4.hpp"

class CameraViewportResolver
{
public:
    virtual ~CameraViewportResolver() = default;

    static ::float4 ResolveViewport(::float4 viewport, double targetWidth, double targetHeight);
private:
    static bool UsesStackedDualScreenViewportUnits(::float4 viewport, double targetWidth, double targetHeight);
};
