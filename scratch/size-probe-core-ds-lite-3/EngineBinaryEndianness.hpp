#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class EngineBinaryEndianness
{
    LittleEndian = 1,
    BigEndian = 2
};
