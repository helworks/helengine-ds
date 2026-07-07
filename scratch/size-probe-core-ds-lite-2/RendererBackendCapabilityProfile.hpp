#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RendererBackendCapabilityProfile
{
public:
    virtual ~RendererBackendCapabilityProfile() = default;

    bool SupportsForwardRendering;

    bool get_SupportsForwardRendering();

    bool SupportsDeferredRendering;

    bool get_SupportsDeferredRendering();

    bool SupportsHdr;

    bool get_SupportsHdr();

    bool SupportsNormalMaps;

    bool get_SupportsNormalMaps();

    int32_t MaximumVisibleLights;

    int32_t get_MaximumVisibleLights();

    int32_t MaximumShadowedLights;

    int32_t get_MaximumShadowedLights();

    RendererBackendCapabilityProfile(bool supportsForwardRendering, bool supportsDeferredRendering, bool supportsHdr, bool supportsNormalMaps, int32_t maximumVisibleLights, int32_t maximumShadowedLights);
};
