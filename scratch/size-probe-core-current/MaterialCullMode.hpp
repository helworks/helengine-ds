#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class MaterialCullMode
{
    None,
    Front,
    Back
};
