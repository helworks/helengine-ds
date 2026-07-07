#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class PlatformMenuSceneResolver
{
public:
    virtual ~PlatformMenuSceneResolver() = default;

    inline static const std::string DesktopMainMenuSceneId = "DemoDiscMainMenu";

    inline static const std::string NintendoDsMainMenuSceneId = "DemoDiscMainMenuDs";

    inline static const std::string GeneratedBootSceneId = "GeneratedBootScene";
};
