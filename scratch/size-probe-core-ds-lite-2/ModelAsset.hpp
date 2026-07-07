#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class float3;
class float2;
class ModelSubmeshAsset;

#include "Asset.hpp"
#include "runtime/array.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"
#include "float2.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class ModelAsset : public ::Asset
{
public:
    virtual ~ModelAsset() = default;

    ModelAsset();

    Array<::float3>* Positions;

    Array<::float3>* Normals;

    Array<::float2>* TexCoords;

    ::float3 BoundsMin;

    ::float3 BoundsMax;

    Array<uint16_t>* Indices16;

    Array<uint32_t>* Indices32;

    Array<::ModelSubmeshAsset*>* Submeshes;

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
};
