#ifdef DrawText
#undef DrawText
#endif
#include "ModelAssetIndexData.hpp"
#include "runtime/native_exceptions.hpp"
#include "ModelAssetIndexData.hpp"
#include "runtime/array.hpp"
#include "ModelAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

bool ModelAssetIndexData::get_Uses32BitIndices()
{
return this->Uses32BitIndices;
}

int32_t ModelAssetIndexData::get_IndexCount()
{
return this->IndexCount;
}

Array<uint16_t>* ModelAssetIndexData::get_Indices16()
{
return this->Indices16;
}

Array<uint32_t>* ModelAssetIndexData::get_Indices32()
{
return this->Indices32;
}

::ModelAssetIndexData* ModelAssetIndexData::Resolve(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
const bool hasIndices16 = ModelAssetIndexData::HasIndices(asset->Indices16);
const bool hasIndices32 = ModelAssetIndexData::HasIndices(asset->Indices32);
    if (hasIndices16 && hasIndices32)
    {
throw new InvalidOperationException("Model assets cannot define both 16-bit and 32-bit index buffers.");
    }
else {
    if (hasIndices32)
    {
return new ::ModelAssetIndexData(true, static_cast<int32_t>(asset->Indices32->get_Length()), nullptr, asset->Indices32);    }
else {
    if (hasIndices16)
    {
return new ::ModelAssetIndexData(false, static_cast<int32_t>(asset->Indices16->get_Length()), asset->Indices16, nullptr);    }
}
}
return new ::ModelAssetIndexData(false, static_cast<int32_t>(0), asset->Indices16, asset->Indices32);}

bool ModelAssetIndexData::HasIndices(Array<uint16_t>* indices)
{
return indices != nullptr && indices->get_Length() > 0;}

bool ModelAssetIndexData::HasIndices(Array<uint32_t>* indices)
{
return indices != nullptr && indices->get_Length() > 0;}

ModelAssetIndexData::ModelAssetIndexData(bool uses32BitIndices, int32_t indexCount, Array<uint16_t>* indices16, Array<uint32_t>* indices32) : Uses32BitIndices(), IndexCount(0), Indices16(), Indices32()
{
this->Uses32BitIndices = uses32BitIndices;
this->IndexCount = indexCount;
this->Indices16 = indices16;
this->Indices32 = indices32;
}

