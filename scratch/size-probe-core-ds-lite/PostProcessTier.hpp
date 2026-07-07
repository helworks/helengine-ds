#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class PostProcessTier
{
    Disabled = 0,
    Low = 1,
    High = 2
};
