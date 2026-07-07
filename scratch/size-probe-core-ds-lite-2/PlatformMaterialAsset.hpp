#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;

#include "Asset.hpp"
#include "runtime/native_string.hpp"

class PlatformMaterialAsset : public ::Asset
{
public:
    virtual ~PlatformMaterialAsset() = default;

    PlatformMaterialAsset();

    std::string RendererFamilyId;

    std::string TextureRelativePath;

    bool DoubleSided;

    bool UseVertexColor;

    bool Lit;

    uint8_t BaseColorR;

    uint8_t BaseColorG;

    uint8_t BaseColorB;

    uint8_t BaseColorA;

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
};
