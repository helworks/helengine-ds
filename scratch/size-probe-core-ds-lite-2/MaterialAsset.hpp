#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class MaterialRenderState;

#include "Asset.hpp"

class MaterialAsset : public ::Asset
{
public:
    virtual ~MaterialAsset() = default;

    ::MaterialRenderState* RenderState;

    bool CastsShadows;

    bool ReceivesShadows;

    MaterialAsset();

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
};
