#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2,
    Ambient = 3
};
