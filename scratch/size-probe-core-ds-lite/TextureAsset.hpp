#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;

#include "Asset.hpp"
#include "runtime/array.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"

class TextureAsset : public ::Asset
{
public:
    virtual ~TextureAsset() = default;

    TextureAsset();

    Array<uint8_t>* Colors;

    Array<uint8_t>* PaletteColors;

    uint16_t Width;

    uint16_t Height;

    ::TextureAssetColorFormat ColorFormat;

    ::TextureAssetAlphaPrecision AlphaPrecision;

    bool IsEngineOwned;

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
};
