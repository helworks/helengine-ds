#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameBatchingMetadata
{
public:
    virtual ~RenderFrameBatchingMetadata() = default;

    bool IsStaticEligible;

    bool get_IsStaticEligible();

    bool IsDynamicEligible;

    bool get_IsDynamicEligible();

    bool IsInstancingEligible;

    bool get_IsInstancingEligible();

    RenderFrameBatchingMetadata(bool isStaticEligible, bool isDynamicEligible, bool isInstancingEligible);
};
