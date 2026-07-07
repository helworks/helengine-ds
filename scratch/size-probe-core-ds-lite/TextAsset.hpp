#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;

#include "Asset.hpp"
#include "runtime/native_string.hpp"

class TextAsset : public ::Asset
{
public:
    virtual ~TextAsset() = default;

    TextAsset();

    std::string Text;

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
};
