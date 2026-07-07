#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class RenderCommand2DType
{
    ClipPush = 1,
    ClipPop = 2,
    TexturedQuad = 3,
    GlyphQuad = 4,
    RoundedRect = 5
};
