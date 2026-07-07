#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class TextureAssetColorFormat
{
    Rgba32 = 0,
    Rgba4444 = 1,
    Indexed4 = 2,
    Indexed8 = 3,
    GxRgb5A3 = 4
};
