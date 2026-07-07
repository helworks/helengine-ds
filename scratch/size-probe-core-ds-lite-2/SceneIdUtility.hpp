#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class SceneIdUtility
{
public:
    virtual ~SceneIdUtility() = default;

    static std::string FromPath(std::string scenePath);
};
