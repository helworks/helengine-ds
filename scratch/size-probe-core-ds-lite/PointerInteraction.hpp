#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class PointerInteraction
{
    None,
    Hover,
    Leave,
    Press,
    Release
};
