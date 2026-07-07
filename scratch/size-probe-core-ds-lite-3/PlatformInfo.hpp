#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class PlatformInfo
{
public:
    virtual ~PlatformInfo() = default;

    std::string Name;

    const std::string& get_Name();

    std::string Version;

    const std::string& get_Version();

    PlatformInfo(std::string name, std::string version);
};
