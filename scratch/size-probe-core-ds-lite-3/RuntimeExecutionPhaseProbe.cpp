#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeExecutionPhaseProbe.hpp"
#include "RuntimeExecutionPhaseProbe.hpp"

int32_t RuntimeExecutionPhaseProbe::CurrentPhaseId = 0;

int32_t RuntimeExecutionPhaseProbe::get_CurrentPhaseId()
{
return RuntimeExecutionPhaseProbe::CurrentPhaseId;
}

void RuntimeExecutionPhaseProbe::set_CurrentPhaseId(int32_t value)
{
RuntimeExecutionPhaseProbe::CurrentPhaseId = value;
}

void RuntimeExecutionPhaseProbe::SetCurrentPhaseId(int32_t phaseId)
{
RuntimeExecutionPhaseProbe::set_CurrentPhaseId(phaseId);
}

