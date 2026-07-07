#ifdef DrawText
#undef DrawText
#endif
#include "CameraRenderSettings.hpp"
#include "DepthPrepassMode.hpp"
#include "PostProcessTier.hpp"
#include "CameraRenderSettings.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"

::DepthPrepassMode CameraRenderSettings::get_DepthPrepassMode()
{
return this->DepthPrepassMode;
}

void CameraRenderSettings::set_DepthPrepassMode(::DepthPrepassMode value)
{
this->DepthPrepassMode = value;
}

float CameraRenderSettings::get_ShadowDistance()
{
return this->ShadowDistance;
}

void CameraRenderSettings::set_ShadowDistance(float value)
{
this->ShadowDistance = value;
}

::PostProcessTier CameraRenderSettings::get_PostProcessTier()
{
return this->PostProcessTier;
}

void CameraRenderSettings::set_PostProcessTier(::PostProcessTier value)
{
this->PostProcessTier = value;
}

CameraRenderSettings::CameraRenderSettings() : DepthPrepassMode(), ShadowDistance(), PostProcessTier()
{
this->set_DepthPrepassMode(DepthPrepassMode::Auto);
this->set_ShadowDistance(50.0f);
this->set_PostProcessTier(PostProcessTier::High);
}

CameraRenderSettings::CameraRenderSettings(::CameraRenderSettings* other) : DepthPrepassMode(), ShadowDistance(), PostProcessTier()
{
    if (other == nullptr)
    {
throw new ArgumentNullException("other");
    }
this->set_DepthPrepassMode(other->DepthPrepassMode);
this->set_ShadowDistance(other->ShadowDistance);
this->set_PostProcessTier(other->PostProcessTier);
}

