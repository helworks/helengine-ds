#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class SceneLoadingEventArgs
{
public:
    virtual ~SceneLoadingEventArgs() = default;

    std::string SceneId;

    const std::string& get_SceneId();

    std::string CookedRelativePath;

    const std::string& get_CookedRelativePath();

    SceneLoadingEventArgs(std::string sceneId, std::string cookedRelativePath);
};
