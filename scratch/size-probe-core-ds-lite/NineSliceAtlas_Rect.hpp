#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class NineSliceAtlas_Rect
{
public:
    NineSliceAtlas_Rect();

    int32_t X;

    int32_t Y;

    int32_t W;

    int32_t H;

    NineSliceAtlas_Rect(int32_t x, int32_t y, int32_t w, int32_t h);
};
