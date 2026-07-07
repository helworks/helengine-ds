#ifdef DrawText
#undef DrawText
#endif
#include "ModelAsset.hpp"
#include "runtime/array.hpp"
#include "float3.hpp"
#include "float2.hpp"
#include "ModelSubmeshAsset.hpp"
#include "runtime/array.hpp"

ModelAsset::ModelAsset() : Positions(), Normals(), TexCoords(), BoundsMin(), BoundsMax(), Indices16(), Indices32(), Submeshes()
{
}

const std::string& ModelAsset::get_Id()
{
return Asset::get_Id();
}

void ModelAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t ModelAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void ModelAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

