#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class Asset
{
public:
    virtual ~Asset() = default;

    Asset();

    std::string Id;

    const std::string& get_Id();
    void set_Id(std::string value);

    uint64_t RuntimeAssetId;

    uint64_t get_RuntimeAssetId();
    void set_RuntimeAssetId(uint64_t value);
};
