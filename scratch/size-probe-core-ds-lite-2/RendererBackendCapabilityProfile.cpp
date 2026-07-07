#ifdef DrawText
#undef DrawText
#endif
#include "RendererBackendCapabilityProfile.hpp"
#include "RendererBackendCapabilityProfile.hpp"

bool RendererBackendCapabilityProfile::get_SupportsForwardRendering()
{
return this->SupportsForwardRendering;
}

bool RendererBackendCapabilityProfile::get_SupportsDeferredRendering()
{
return this->SupportsDeferredRendering;
}

bool RendererBackendCapabilityProfile::get_SupportsHdr()
{
return this->SupportsHdr;
}

bool RendererBackendCapabilityProfile::get_SupportsNormalMaps()
{
return this->SupportsNormalMaps;
}

int32_t RendererBackendCapabilityProfile::get_MaximumVisibleLights()
{
return this->MaximumVisibleLights;
}

int32_t RendererBackendCapabilityProfile::get_MaximumShadowedLights()
{
return this->MaximumShadowedLights;
}

RendererBackendCapabilityProfile::RendererBackendCapabilityProfile(bool supportsForwardRendering, bool supportsDeferredRendering, bool supportsHdr, bool supportsNormalMaps, int32_t maximumVisibleLights, int32_t maximumShadowedLights) : SupportsForwardRendering(), SupportsDeferredRendering(), SupportsHdr(), SupportsNormalMaps(), MaximumVisibleLights(0), MaximumShadowedLights(0)
{
this->SupportsForwardRendering = supportsForwardRendering;
this->SupportsDeferredRendering = supportsDeferredRendering;
this->SupportsHdr = supportsHdr;
this->SupportsNormalMaps = supportsNormalMaps;
this->MaximumVisibleLights = maximumVisibleLights;
this->MaximumShadowedLights = maximumShadowedLights;
}

