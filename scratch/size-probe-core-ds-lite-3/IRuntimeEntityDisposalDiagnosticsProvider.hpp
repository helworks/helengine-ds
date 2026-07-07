#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class IRuntimeEntityDisposalDiagnosticsProvider
{
public:
    virtual void ReportEntityDisposalStage(std::string stage, int32_t entityChildCount, int32_t componentCount, int32_t componentIndex) = 0;
};
