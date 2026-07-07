#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeMemoryCounters;

class IRuntimeMemoryCounterProvider
{
public:
    virtual void CaptureMemoryCounters(::RuntimeMemoryCounters* counters) = 0;
};
