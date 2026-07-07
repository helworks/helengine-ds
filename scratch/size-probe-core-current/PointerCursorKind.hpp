#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class PointerCursorKind
{
    Default,
    Hand,
    Text,
    ResizeNorthWestSouthEast,
    ResizeNorthEastSouthWest
};
