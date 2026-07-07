#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class ComponentExecutionMode
{
    Runtime = 0,
    Editor = 1
};
