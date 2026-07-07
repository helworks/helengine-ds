#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class SceneMemoryProbeActionKind
{
    Wait = 0,
    LoadSceneSingle = 1,
    LoadSceneAdditive = 2,
    UnloadScene = 3
};
