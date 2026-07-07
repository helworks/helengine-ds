#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class RenderPassKind
{
    DepthPrepass,
    Shadow,
    OpaqueForward,
    TransparentForward,
    PostProcess,
    Present
};
