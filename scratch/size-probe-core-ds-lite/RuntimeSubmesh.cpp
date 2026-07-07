#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSubmesh.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

RuntimeSubmesh::RuntimeSubmesh() : MaterialSlotName(String::Empty), IndexStart(0), IndexCount(0)
{
}

const std::string& RuntimeSubmesh::get_MaterialSlotName()
{
return this->MaterialSlotName;
}

void RuntimeSubmesh::set_MaterialSlotName(std::string value)
{
this->MaterialSlotName = value;
}

int32_t RuntimeSubmesh::get_IndexStart()
{
return this->IndexStart;
}

void RuntimeSubmesh::set_IndexStart(int32_t value)
{
this->IndexStart = value;
}

int32_t RuntimeSubmesh::get_IndexCount()
{
return this->IndexCount;
}

void RuntimeSubmesh::set_IndexCount(int32_t value)
{
this->IndexCount = value;
}

