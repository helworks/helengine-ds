#ifdef DrawText
#undef DrawText
#endif
#include "TextAsset.hpp"
#include "runtime/native_string.hpp"

TextAsset::TextAsset() : Text()
{
}

const std::string& TextAsset::get_Id()
{
return Asset::get_Id();
}

void TextAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t TextAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void TextAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

