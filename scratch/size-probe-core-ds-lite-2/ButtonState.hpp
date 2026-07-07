#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class ButtonState
{
    Released,
    Pressed,
    JustReleased,
    JustPressed
};
