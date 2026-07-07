#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;

#include "system/io/stream.hpp"
#include "runtime/array.hpp"

class AssetSerializer
{
public:
    virtual ~AssetSerializer() = default;

    static ::Asset* Deserialize(::Stream* stream);

    static ::Asset* DeserializeFromBytes(Array<uint8_t>* data);
};
