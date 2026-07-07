#ifdef DrawText
#undef DrawText
#endif
#include "PlatformMaterialAsset.hpp"
#include "runtime/native_string.hpp"

PlatformMaterialAsset::PlatformMaterialAsset() : RendererFamilyId(), TextureRelativePath(), DoubleSided(), UseVertexColor(), Lit(), BaseColorR(), BaseColorG(), BaseColorB(), BaseColorA()
{
}

const std::string& PlatformMaterialAsset::get_Id()
{
return Asset::get_Id();
}

void PlatformMaterialAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t PlatformMaterialAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void PlatformMaterialAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

