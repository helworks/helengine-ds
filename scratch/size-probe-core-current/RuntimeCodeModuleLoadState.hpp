#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class RuntimeCodeModuleLoadState
{
    ResidentAtStartup = 0,
    SceneResident = 1,
    Unloadable = 2
};
