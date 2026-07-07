#ifdef DrawText
#undef DrawText
#endif
#include "TextureAsset.hpp"
#include "runtime/array.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "runtime/array.hpp"

TextureAsset::TextureAsset() : Colors(), PaletteColors(), Width(), Height(), ColorFormat(), AlphaPrecision(), IsEngineOwned()
{
}

const std::string& TextureAsset::get_Id()
{
return Asset::get_Id();
}

void TextureAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t TextureAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void TextureAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

