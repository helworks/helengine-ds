#ifdef DrawText
#undef DrawText
#endif
#include "MaterialAsset.hpp"
#include "MaterialRenderState.hpp"
#include "MaterialBlendMode.hpp"
#include "MaterialCullMode.hpp"
#include "MaterialAsset.hpp"

MaterialAsset::MaterialAsset() : RenderState(), CastsShadows(), ReceivesShadows()
{
this->RenderState = new ::MaterialRenderState();
this->CastsShadows = true;
this->ReceivesShadows = true;
}

const std::string& MaterialAsset::get_Id()
{
return Asset::get_Id();
}

void MaterialAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t MaterialAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void MaterialAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

