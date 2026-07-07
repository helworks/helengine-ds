#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeMemoryDiagnosticsSnapshot;

class IRuntimeDiagnosticsProvider
{
public:
    virtual ::RuntimeMemoryDiagnosticsSnapshot* CaptureSnapshot() = 0;
};
