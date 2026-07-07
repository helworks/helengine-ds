#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameBatchingMetadata.hpp"
#include "RenderFrameBatchingMetadata.hpp"

bool RenderFrameBatchingMetadata::get_IsStaticEligible()
{
return this->IsStaticEligible;
}

bool RenderFrameBatchingMetadata::get_IsDynamicEligible()
{
return this->IsDynamicEligible;
}

bool RenderFrameBatchingMetadata::get_IsInstancingEligible()
{
return this->IsInstancingEligible;
}

RenderFrameBatchingMetadata::RenderFrameBatchingMetadata(bool isStaticEligible, bool isDynamicEligible, bool isInstancingEligible) : IsStaticEligible(), IsDynamicEligible(), IsInstancingEligible()
{
this->IsStaticEligible = isStaticEligible;
this->IsDynamicEligible = isDynamicEligible;
this->IsInstancingEligible = isInstancingEligible;
}

