#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class EngineBinaryHeader;

#include "system/io/stream.hpp"
#include "EngineBinaryEndianness.hpp"

class EngineBinaryHeaderSerializer
{
public:
    virtual ~EngineBinaryHeaderSerializer() = default;

    static ::EngineBinaryHeader* Read(::Stream* stream);

    static void Write(::Stream* stream, ::EngineBinaryHeader* header);
private:
    static void ValidateEndianness(::EngineBinaryEndianness endianness);
};
