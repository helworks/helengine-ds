#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class RendererFeatureDowngradeMode
{
    Required,
    Degrade,
    Drop
};
