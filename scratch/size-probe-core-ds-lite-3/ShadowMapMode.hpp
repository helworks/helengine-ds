#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class ShadowMapMode
{
    Disabled = 0,
    Auto = 1,
    Forced = 2
};
