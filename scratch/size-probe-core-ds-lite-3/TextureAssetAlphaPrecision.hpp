#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class TextureAssetAlphaPrecision
{
    Opaque = 0,
    Binary = 1,
    A4 = 2,
    A8 = 3
};
