#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class AnimationClipPlatformOverrideMode
{
    InheritBase = 0,
    ReplaceWholeClip = 1,
    OverrideFrames = 2
};
