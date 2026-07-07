#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "DepthPrepassMode.hpp"
#include "PostProcessTier.hpp"

class CameraRenderSettings
{
public:
    virtual ~CameraRenderSettings() = default;

    ::DepthPrepassMode DepthPrepassMode;

    ::DepthPrepassMode get_DepthPrepassMode();
    void set_DepthPrepassMode(::DepthPrepassMode value);

    float ShadowDistance;

    float get_ShadowDistance();
    void set_ShadowDistance(float value);

    ::PostProcessTier PostProcessTier;

    ::PostProcessTier get_PostProcessTier();
    void set_PostProcessTier(::PostProcessTier value);

    CameraRenderSettings();

    CameraRenderSettings(::CameraRenderSettings* other);
};
