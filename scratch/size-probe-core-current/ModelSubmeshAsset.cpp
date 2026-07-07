#ifdef DrawText
#undef DrawText
#endif
#include "ModelSubmeshAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

ModelSubmeshAsset::ModelSubmeshAsset() : MaterialSlotName(String::Empty), IndexStart(0), IndexCount(0)
{
}

const std::string& ModelSubmeshAsset::get_MaterialSlotName()
{
return this->MaterialSlotName;
}

void ModelSubmeshAsset::set_MaterialSlotName(std::string value)
{
this->MaterialSlotName = value;
}

int32_t ModelSubmeshAsset::get_IndexStart()
{
return this->IndexStart;
}

void ModelSubmeshAsset::set_IndexStart(int32_t value)
{
this->IndexStart = value;
}

int32_t ModelSubmeshAsset::get_IndexCount()
{
return this->IndexCount;
}

void ModelSubmeshAsset::set_IndexCount(int32_t value)
{
this->IndexCount = value;
}

