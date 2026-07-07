#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneMemoryProbeMeasurement;

#include "runtime/native_string.hpp"
#include "SceneMemoryProbeActionKind.hpp"

class SceneMemoryProbeLogFormatter
{
public:
    virtual ~SceneMemoryProbeLogFormatter() = default;

    static std::string Format(::SceneMemoryProbeMeasurement* measurement);
private:
    static std::string FormatActionKind(::SceneMemoryProbeActionKind actionKind);

    static std::string FormatInt32(int32_t value);

    static std::string FormatUInt64(uint64_t value);
};
