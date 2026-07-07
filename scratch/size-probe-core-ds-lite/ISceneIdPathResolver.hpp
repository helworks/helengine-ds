#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class ISceneIdPathResolver
{
public:
    virtual std::string ResolveScenePath(std::string sceneId) = 0;
};
