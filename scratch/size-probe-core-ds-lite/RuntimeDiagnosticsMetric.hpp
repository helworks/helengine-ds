#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class RuntimeDiagnosticsMetric
{
public:
    virtual ~RuntimeDiagnosticsMetric() = default;

    std::string Name;

    const std::string& get_Name();

    uint64_t Value;

    uint64_t get_Value();

    RuntimeDiagnosticsMetric(std::string name, uint64_t value);
};
