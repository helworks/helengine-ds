#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class DepthPrepassMode
{
    Auto = 0,
    Disabled = 1,
    Always = 2
};
