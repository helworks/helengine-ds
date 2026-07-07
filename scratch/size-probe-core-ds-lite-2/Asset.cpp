#ifdef DrawText
#undef DrawText
#endif
#include "Asset.hpp"
#include "runtime/native_string.hpp"

Asset::Asset() : Id(), RuntimeAssetId()
{
}

const std::string& Asset::get_Id()
{
return this->Id;
}

void Asset::set_Id(std::string value)
{
this->Id = value;
}

uint64_t Asset::get_RuntimeAssetId()
{
return this->RuntimeAssetId;
}

void Asset::set_RuntimeAssetId(uint64_t value)
{
this->RuntimeAssetId = value;
}

