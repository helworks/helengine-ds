#ifdef DrawText
#undef DrawText
#endif
#include "AssetSerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "Asset.hpp"
#include "system/io/memory-stream.hpp"
#include "EditorAssetBinarySerializer.hpp"
#include "AssetSerializer.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "EngineBinaryEndianness.hpp"
#include "runtime/native_string.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "SceneAsset.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "TextureAssetColorFormat.hpp"
#include "AnimationClipAsset.hpp"
#include "EngineBinaryReader.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "AnimationInterpolationMode.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityAsset.hpp"
#include "MaterialAsset.hpp"
#include "MaterialRenderState.hpp"
#include "ModelAsset.hpp"
#include "ModelSubmeshAsset.hpp"
#include "PlatformMaterialAsset.hpp"
#include "PlatformPositionKeyframeTrackAsset.hpp"
#include "PlatformRotationKeyframeTrackAsset.hpp"
#include "PositionKeyframeAsset.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "RotationKeyframeAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "SceneAssetReference.hpp"
#include "SceneCanvasProfile.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "TextAsset.hpp"
#include "TextureAsset.hpp"
#include "NativeOwnership.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "system/io/stream.hpp"

::Asset* AssetSerializer::Deserialize(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
{
auto __finallyGuard_0000001B = he_cpp_make_scope_exit([&]() {
delete header;
});
    if (header->FormatId == EditorAssetBinarySerializer::FormatId)
    {
return EditorAssetBinarySerializer::Deserialize(stream, header);    }
throw new InvalidOperationException(std::string("Unsupported asset binary format id '") + std::to_string(header->FormatId) + std::string("'."));
}
}

::Asset* AssetSerializer::DeserializeFromBytes(Array<uint8_t>* data)
{
    if (data == nullptr)
    {
throw new ArgumentNullException("data");
    }
{
::MemoryStream *stream = new ::MemoryStream(data, false);
auto __usingDisposeGuard_0000001C = he_cpp_make_scope_exit([&]() {
if (stream != nullptr) {
stream->Dispose();
delete stream;
}
});
return AssetSerializer::Deserialize(stream);}
}

