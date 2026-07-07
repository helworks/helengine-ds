#ifdef DrawText
#undef DrawText
#endif
#include "AssetContentProcessor_1.hpp"
#include "runtime/native_exceptions.hpp"
#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "runtime/native_string.hpp"
#include "EngineBinaryReadContext.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "AssetContentProcessor_1.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"

template <typename TAsset>
Type* AssetContentProcessor_1<TAsset>::get_OutputType()
{
return he_cpp_type_of<TAsset>("TAsset");
}

template <typename TAsset>
TAsset AssetContentProcessor_1<TAsset>::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::Asset *asset = AssetSerializer::Deserialize(stream);
    TAsset typedAsset = he_cpp_try_cast<TAsset>(asset);
    if (typedAsset != nullptr)
    {
return typedAsset;    }
const std::string actualAssetTypeName = asset == nullptr ? "<null>" : he_cpp_type_of<Asset>("Asset")->Name;
const std::string activeAssetPath = EngineBinaryReadContext::get_CurrentAssetPath();
const std::string activeReadStage = EngineBinaryReadContext::get_CurrentReadStage();
const std::string lastCheckpoint = EngineBinaryReadContext::get_LastCheckpoint();
throw new InvalidOperationException(std::string("Serialized asset did not contain '") + he_cpp_type_of<TAsset>("TAsset")->get_Name() + std::string("'. Actual asset type was '") + actualAssetTypeName + std::string("'. asset_path='") + activeAssetPath + std::string("' read_stage='") + activeReadStage + std::string("' last_checkpoint='") + lastCheckpoint + std::string("'."));
}

template <typename TAsset>
void* AssetContentProcessor_1<TAsset>::ReadObject(::Stream* stream)
{
return this->Read(stream);}

