#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class InputControlKind
{
    Button,
    Axis,
    PointerDelta,
    ScrollWheel
};
