#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class IRuntimeUpdateStageDiagnosticsProvider
{
public:
    virtual void ReportUpdateStage(std::string stage) = 0;
};
