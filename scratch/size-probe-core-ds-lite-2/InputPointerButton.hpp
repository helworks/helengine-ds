#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class InputPointerButton
{
    Primary = 0,
    Secondary = 1,
    Middle = 2,
    Back = 3,
    Forward = 4
};
