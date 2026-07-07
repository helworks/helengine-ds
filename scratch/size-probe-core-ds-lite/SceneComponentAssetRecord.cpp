#ifdef DrawText
#undef DrawText
#endif
#include "SceneComponentAssetRecord.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "runtime/array.hpp"

int32_t SceneComponentAssetRecord::get_LiveInstanceCount()
{
return LiveInstanceCountValue;
}

const std::string& SceneComponentAssetRecord::get_ComponentKey()
{
return this->ComponentKey;
}

void SceneComponentAssetRecord::set_ComponentKey(std::string value)
{
this->ComponentKey = value;
}

const std::string& SceneComponentAssetRecord::get_ComponentTypeId()
{
return this->ComponentTypeId;
}

void SceneComponentAssetRecord::set_ComponentTypeId(std::string value)
{
this->ComponentTypeId = value;
}

int32_t SceneComponentAssetRecord::get_ComponentIndex()
{
return this->ComponentIndex;
}

void SceneComponentAssetRecord::set_ComponentIndex(int32_t value)
{
this->ComponentIndex = value;
}

Array<uint8_t>* SceneComponentAssetRecord::get_Payload()
{
return this->Payload;
}

void SceneComponentAssetRecord::set_Payload(Array<uint8_t>* value)
{
this->Payload = value;
}

void SceneComponentAssetRecord::MarkReleasedForDiagnostics()
{
LiveInstanceCountValue--;
}

SceneComponentAssetRecord::SceneComponentAssetRecord() : ComponentKey(), ComponentTypeId(), ComponentIndex(0), Payload(Array<uint8_t>::Empty())
{
LiveInstanceCountValue++;
}

int32_t SceneComponentAssetRecord::LiveInstanceCountValue = 0;

