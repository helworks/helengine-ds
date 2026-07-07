#ifdef DrawText
#undef DrawText
#endif
#include "EditorAssetBinarySerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "Asset.hpp"
#include "SceneAsset.hpp"
#include "EngineBinaryReader.hpp"
#include "TextureAsset.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "ModelAsset.hpp"
#include "ModelSubmeshAsset.hpp"
#include "TextAsset.hpp"
#include "MaterialAsset.hpp"
#include "MaterialRenderState.hpp"
#include "NativeOwnership.hpp"
#include "PlatformMaterialAsset.hpp"
#include "AnimationClipAsset.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "PlatformPositionKeyframeTrackAsset.hpp"
#include "PlatformRotationKeyframeTrackAsset.hpp"
#include "PositionKeyframeAsset.hpp"
#include "RotationKeyframeAsset.hpp"
#include "AnimationInterpolationMode.hpp"
#include "SceneSettingsAsset.hpp"
#include "SceneCanvasProfile.hpp"
#include "runtime/native_string.hpp"
#include "SceneEntityAsset.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneAssetReference.hpp"
#include "float2.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "EditorAssetBinarySerializer.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "BinaryReaderLE.hpp"
#include "BinaryReaderBE.hpp"
#include "SceneAssetReferenceFactory.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/func.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "SceneEntityReference.hpp"
#include "runtime/native_span.hpp"
#include "MaterialBlendMode.hpp"
#include "MaterialCullMode.hpp"
#include "AnimationClipPlatformOverrideMode.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "EngineBinaryReadContext.hpp"
#include "BinaryReaderBE.hpp"
#include "BinaryReaderLE.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_span.hpp"
#include "runtime/native_string.hpp"
#include "system/io/stream.hpp"

const ::EditorBinaryRecordKind EditorAssetBinarySerializer::RecordKind = EditorBinaryRecordKind::Asset;

::Asset* EditorAssetBinarySerializer::Deserialize(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
{
auto __finallyGuard_00000055 = he_cpp_make_scope_exit([&]() {
delete header;
});
return EditorAssetBinarySerializer::Deserialize(stream, header);}
}

::Asset* EditorAssetBinarySerializer::Deserialize(::Stream* stream, ::EngineBinaryHeader* header)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else {
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
}
EngineBinaryReadContext::set_CurrentReadStage("EditorAssetBinarySerializer:ValidateHeader");
EditorAssetBinarySerializer::ValidateHeader(header);
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, static_cast<EngineBinaryEndianness>(header->Endianness), true);
auto __usingDisposeGuard_00000056 = he_cpp_make_scope_exit([&]() {
if (reader != nullptr) {
reader->Dispose();
delete reader;
}
});
EngineBinaryReadContext::set_CurrentReadStage("EditorAssetBinarySerializer:ReadAssetPayload");
return EditorAssetBinarySerializer::ReadAssetPayload(reader, static_cast<EditorAssetBinaryValueKind>(static_cast<EditorAssetBinaryValueKind>(header->ValueKind)), static_cast<uint8_t>(header->Version));}
}

::SceneAsset* EditorAssetBinarySerializer::DeserializeSceneAsset(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
{
auto __finallyGuard_00000057 = he_cpp_make_scope_exit([&]() {
delete header;
});
return EditorAssetBinarySerializer::DeserializeSceneAsset(stream, header);}
}

::SceneAsset* EditorAssetBinarySerializer::DeserializeSceneAsset(::Stream* stream, ::EngineBinaryHeader* header)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else {
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
}
EngineBinaryReadContext::set_CurrentReadStage("EditorAssetBinarySerializer:ValidateHeader");
EditorAssetBinarySerializer::ValidateHeader(header);
    if (static_cast<EditorAssetBinaryValueKind>(header->ValueKind) != EditorAssetBinaryValueKind::SceneAsset)
    {
throw new InvalidOperationException(std::string("Serialized payload value kind '") + std::to_string(header->ValueKind) + std::string("' is not supported for scene-asset deserialization."));
    }
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, static_cast<EngineBinaryEndianness>(header->Endianness), true);
auto __usingDisposeGuard_00000058 = he_cpp_make_scope_exit([&]() {
if (reader != nullptr) {
reader->Dispose();
delete reader;
}
});
EngineBinaryReadContext::set_CurrentReadStage("EditorAssetBinarySerializer:ReadAssetPayload");
return EditorAssetBinarySerializer::ReadSceneAsset(reader, static_cast<uint8_t>(header->Version));}
}

::TextureAssetAlphaPrecision EditorAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(::TextureAssetColorFormat colorFormat)
{
    if (colorFormat == TextureAssetColorFormat::Rgba4444)
    {
return TextureAssetAlphaPrecision::A4;    }
return TextureAssetAlphaPrecision::A8;}

::AnimationClipAsset* EditorAssetBinarySerializer::ReadAnimationClipAsset(::EngineBinaryReader* reader, uint8_t version)
{
::AnimationClipAsset *asset = new ::AnimationClipAsset();
EditorAssetBinarySerializer::ReadAssetIdentity(reader, asset, static_cast<uint8_t>(version));
asset->set_Duration(reader->ReadSingle());
asset->set_PositionTracks(([&]() {
auto __delegateArg_00000059 = new Func<EngineBinaryReader*, PositionKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPositionKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_0000005A = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000059;
});
Array<::PositionKeyframeTrackAsset*>* __coalesce_0000005B = ([&]() -> Array<::PositionKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeTrackAsset*>(__delegateArg_00000059);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeTrackAsset*>(__delegateArg_00000059);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_0000005B != nullptr ? __coalesce_0000005B : Array<PositionKeyframeTrackAsset*>::Empty();
})());
asset->set_PositionOffsetTracks(([&]() {
auto __delegateArg_0000005C = new Func<EngineBinaryReader*, PositionOffsetKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPositionOffsetKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_0000005D = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000005C;
});
Array<::PositionOffsetKeyframeTrackAsset*>* __coalesce_0000005E = ([&]() -> Array<::PositionOffsetKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionOffsetKeyframeTrackAsset*>(__delegateArg_0000005C);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionOffsetKeyframeTrackAsset*>(__delegateArg_0000005C);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_0000005E != nullptr ? __coalesce_0000005E : Array<PositionOffsetKeyframeTrackAsset*>::Empty();
})());
asset->set_ScaleTracks(([&]() {
auto __delegateArg_0000005F = new Func<EngineBinaryReader*, ScaleKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadScaleKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_00000060 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000005F;
});
Array<::ScaleKeyframeTrackAsset*>* __coalesce_00000061 = ([&]() -> Array<::ScaleKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<ScaleKeyframeTrackAsset*>(__delegateArg_0000005F);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<ScaleKeyframeTrackAsset*>(__delegateArg_0000005F);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_00000061 != nullptr ? __coalesce_00000061 : Array<ScaleKeyframeTrackAsset*>::Empty();
})());
asset->set_RotationTracks(([&]() {
auto __delegateArg_00000062 = new Func<EngineBinaryReader*, RotationKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadRotationKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_00000063 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000062;
});
Array<::RotationKeyframeTrackAsset*>* __coalesce_00000064 = ([&]() -> Array<::RotationKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<RotationKeyframeTrackAsset*>(__delegateArg_00000062);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<RotationKeyframeTrackAsset*>(__delegateArg_00000062);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_00000064 != nullptr ? __coalesce_00000064 : Array<RotationKeyframeTrackAsset*>::Empty();
})());
asset->set_PlatformOverrides(version >= AnimationClipPlatformOverrideVersion ? ([&]() {
auto __delegateArg_00000065 = new Func<EngineBinaryReader*, AnimationClipPlatformOverrideAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadAnimationClipPlatformOverrideAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_00000066 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000065;
});
Array<::AnimationClipPlatformOverrideAsset*>* __coalesce_00000067 = ([&]() -> Array<::AnimationClipPlatformOverrideAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<AnimationClipPlatformOverrideAsset*>(__delegateArg_00000065);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<AnimationClipPlatformOverrideAsset*>(__delegateArg_00000065);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_00000067 != nullptr ? __coalesce_00000067 : Array<AnimationClipPlatformOverrideAsset*>::Empty();
})() : Array<AnimationClipPlatformOverrideAsset*>::Empty());
return asset;}

::AnimationClipPlatformOverrideAsset* EditorAssetBinarySerializer::ReadAnimationClipPlatformOverrideAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_00000068 = new ::AnimationClipPlatformOverrideAsset();
__object_00000068->set_PlatformId(reader->ReadString());
__object_00000068->set_Mode(static_cast<AnimationClipPlatformOverrideMode>(reader->ReadByte()));
__object_00000068->set_PositionTracks(([&]() {
auto __delegateArg_00000069 = new Func<EngineBinaryReader*, PlatformPositionKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPlatformPositionKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_0000006A = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000069;
});
Array<::PlatformPositionKeyframeTrackAsset*>* __coalesce_0000006B = ([&]() -> Array<::PlatformPositionKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformPositionKeyframeTrackAsset*>(__delegateArg_00000069);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformPositionKeyframeTrackAsset*>(__delegateArg_00000069);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_0000006B != nullptr ? __coalesce_0000006B : Array<PlatformPositionKeyframeTrackAsset*>::Empty();
})());
__object_00000068->set_PositionOffsetTracks(([&]() {
auto __delegateArg_0000006C = new Func<EngineBinaryReader*, PlatformPositionKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPlatformPositionKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_0000006D = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000006C;
});
Array<::PlatformPositionKeyframeTrackAsset*>* __coalesce_0000006E = ([&]() -> Array<::PlatformPositionKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformPositionKeyframeTrackAsset*>(__delegateArg_0000006C);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformPositionKeyframeTrackAsset*>(__delegateArg_0000006C);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_0000006E != nullptr ? __coalesce_0000006E : Array<PlatformPositionKeyframeTrackAsset*>::Empty();
})());
__object_00000068->set_ScaleTracks(([&]() {
auto __delegateArg_0000006F = new Func<EngineBinaryReader*, PlatformPositionKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPlatformPositionKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_00000070 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000006F;
});
Array<::PlatformPositionKeyframeTrackAsset*>* __coalesce_00000071 = ([&]() -> Array<::PlatformPositionKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformPositionKeyframeTrackAsset*>(__delegateArg_0000006F);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformPositionKeyframeTrackAsset*>(__delegateArg_0000006F);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_00000071 != nullptr ? __coalesce_00000071 : Array<PlatformPositionKeyframeTrackAsset*>::Empty();
})());
__object_00000068->set_RotationTracks(([&]() {
auto __delegateArg_00000072 = new Func<EngineBinaryReader*, PlatformRotationKeyframeTrackAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPlatformRotationKeyframeTrackAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_00000073 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000072;
});
Array<::PlatformRotationKeyframeTrackAsset*>* __coalesce_00000074 = ([&]() -> Array<::PlatformRotationKeyframeTrackAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformRotationKeyframeTrackAsset*>(__delegateArg_00000072);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PlatformRotationKeyframeTrackAsset*>(__delegateArg_00000072);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_00000074 != nullptr ? __coalesce_00000074 : Array<PlatformRotationKeyframeTrackAsset*>::Empty();
})());
return __object_00000068;
})();}

::AnimationInterpolationMode EditorAssetBinarySerializer::ReadAnimationInterpolationMode(::EngineBinaryReader* reader)
{
return static_cast<AnimationInterpolationMode>(reader->ReadByte());}

void EditorAssetBinarySerializer::ReadAssetIdentity(::EngineBinaryReader* reader, ::Asset* asset, uint8_t version)
{
asset->set_Id(reader->ReadString());
asset->set_RuntimeAssetId(version > PreviousVersionWithoutRuntimeAssetId ? static_cast<uint64_t>(reader->ReadInt64()) : 0ul);
}

::Asset* EditorAssetBinarySerializer::ReadAssetPayload(::EngineBinaryReader* reader, ::EditorAssetBinaryValueKind valueKind, uint8_t version)
{
switch (valueKind) {
case EditorAssetBinaryValueKind::TextureAsset: {
return EditorAssetBinarySerializer::ReadTextureAsset(reader, static_cast<uint8_t>(version));}
case EditorAssetBinaryValueKind::ModelAsset: {
return EditorAssetBinarySerializer::ReadModelAsset(reader, static_cast<uint8_t>(version));}
case EditorAssetBinaryValueKind::TextAsset: {
return EditorAssetBinarySerializer::ReadTextAsset(reader, static_cast<uint8_t>(version));}
case EditorAssetBinaryValueKind::MaterialAsset: {
return EditorAssetBinarySerializer::ReadMaterialAsset(reader, static_cast<uint8_t>(version));}
case EditorAssetBinaryValueKind::AnimationClipAsset: {
return EditorAssetBinarySerializer::ReadAnimationClipAsset(reader, static_cast<uint8_t>(version));}
case EditorAssetBinaryValueKind::PlatformMaterialAsset: {
return EditorAssetBinarySerializer::ReadPlatformMaterialAsset(reader, static_cast<uint8_t>(version));}
case EditorAssetBinaryValueKind::SceneAsset: {
return EditorAssetBinarySerializer::ReadSceneAsset(reader, static_cast<uint8_t>(version));}
default:  {
throw new InvalidOperationException(std::string("Unsupported asset value kind '") + std::to_string(static_cast<uint16_t>(valueKind)) + std::string("'."));
}
}

}

bool EditorAssetBinarySerializer::ReadBooleanByte(::EngineBinaryReader* reader, std::string context)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (String::IsNullOrWhiteSpace(context))
    {
throw ([&]() {
auto __ctor_arg_00000075 = "Boolean read context is required.";
auto __ctor_arg_00000076 = "context";
return new ArgumentException(__ctor_arg_00000075, __ctor_arg_00000076);
})();
    }
const uint8_t value = reader->ReadByte();
    if (value == 0)
    {
return false;    }
    if (value == 1)
    {
return true;    }
throw new InvalidOperationException(std::string("Unsupported ") + context + std::string(" boolean value '") + std::to_string(value) + std::string("'."));
}

::float2 EditorAssetBinarySerializer::ReadFloat2(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000077 = reader->ReadSingle();
auto __ctor_arg_00000078 = reader->ReadSingle();
return ::float2(__ctor_arg_00000077, __ctor_arg_00000078);
})();}

::float3 EditorAssetBinarySerializer::ReadFloat3(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000079 = reader->ReadSingle();
auto __ctor_arg_0000007A = reader->ReadSingle();
auto __ctor_arg_0000007B = reader->ReadSingle();
return ::float3(__ctor_arg_00000079, __ctor_arg_0000007A, __ctor_arg_0000007B);
})();}

::float4 EditorAssetBinarySerializer::ReadFloat4(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_0000007C = reader->ReadSingle();
auto __ctor_arg_0000007D = reader->ReadSingle();
auto __ctor_arg_0000007E = reader->ReadSingle();
auto __ctor_arg_0000007F = reader->ReadSingle();
return ::float4(__ctor_arg_0000007C, __ctor_arg_0000007D, __ctor_arg_0000007E, __ctor_arg_0000007F);
})();}

Array<uint8_t>* EditorAssetBinarySerializer::ReadLegacyMaterialConstantBufferAsset(::EngineBinaryReader* reader)
{
reader->ReadString();
return reader->ReadByteArray();}

::SceneComponentAssetRecord* EditorAssetBinarySerializer::ReadLegacySceneComponentAssetRecord(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_00000080 = new ::SceneComponentAssetRecord();
__object_00000080->set_ComponentKey(String::Empty);
__object_00000080->set_ComponentTypeId(reader->ReadString());
__object_00000080->set_ComponentIndex(reader->ReadInt32());
__object_00000080->set_Payload(([&]() {
Array<uint8_t>* __coalesce_00000081 = reader->ReadByteArray();
return __coalesce_00000081 != nullptr ? __coalesce_00000081 : Array<uint8_t>::Empty();
})());
return __object_00000080;
})();}

::SceneEntityAsset* EditorAssetBinarySerializer::ReadLegacySceneEntityAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_00000082 = new ::SceneEntityAsset();
__object_00000082->set_Id(0u);
__object_00000082->set_Name(reader->ReadString());
__object_00000082->set_LocalPosition(reader->ReadFloat3());
__object_00000082->set_LocalScale(reader->ReadFloat3());
__object_00000082->set_LocalOrientation(reader->ReadFloat4());
__object_00000082->set_Components(([&]() {
auto __delegateArg_00000083 = new Func<EngineBinaryReader*, SceneComponentAssetRecord*>(static_cast<SceneComponentAssetRecord* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadLegacySceneComponentAssetRecord));
auto __delegateArgDeleteGuard_00000084 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000083;
});
Array<::SceneComponentAssetRecord*>* __coalesce_00000085 = ([&]() -> Array<::SceneComponentAssetRecord*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneComponentAssetRecord*>(__delegateArg_00000083);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneComponentAssetRecord*>(__delegateArg_00000083);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_00000085 != nullptr ? __coalesce_00000085 : Array<SceneComponentAssetRecord*>::Empty();
})());
__object_00000082->set_PlatformExistenceOverrides(Array<SceneEntityPlatformExistenceOverrideAsset*>::Empty());
__object_00000082->set_PlatformTransformOverrides(Array<SceneEntityPlatformTransformOverrideAsset*>::Empty());
__object_00000082->set_PlatformComponentOverrides(Array<SceneEntityPlatformComponentOverrideAsset*>::Empty());
__object_00000082->set_Children(([&]() {
Array<::SceneEntityAsset*>* __coalesce_00000086 = EditorAssetBinarySerializer::ReadLegacySceneEntityAssetArray(reader);
return __coalesce_00000086 != nullptr ? __coalesce_00000086 : Array<SceneEntityAsset*>::Empty();
})());
return __object_00000082;
})();}

Array<::SceneEntityAsset*>* EditorAssetBinarySerializer::ReadLegacySceneEntityAssetArray(::EngineBinaryReader* reader)
{
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else {
    if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
else {
    if (length == 0)
    {
return Array<SceneEntityAsset*>::Empty();    }
}
}
Array<::SceneEntityAsset*> *values = new Array<SceneEntityAsset*>(length);
for (int32_t index = 0; index < values->get_Length(); index++) {
(*values)[index] = EditorAssetBinarySerializer::ReadLegacySceneEntityAsset(reader);
}
return values;}

::MaterialAsset* EditorAssetBinarySerializer::ReadMaterialAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version < PreviousVersionWithoutRuntimeAssetId || version > CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
}
::MaterialAsset *materialAsset = new ::MaterialAsset();
EditorAssetBinarySerializer::ReadAssetIdentity(reader, materialAsset, static_cast<uint8_t>(version));
    if (version <= LegacyMaterialFieldVersion)
    {
reader->ReadString();
reader->ReadString();
reader->ReadString();
reader->ReadString();
reader->ReadString();
    }
materialAsset->CastsShadows = reader->ReadByte() != 0;
materialAsset->ReceivesShadows = reader->ReadByte() != 0;
::MaterialRenderState *defaultRenderState = materialAsset->RenderState;
materialAsset->RenderState = EditorAssetBinarySerializer::ReadMaterialRenderState(reader);
delete defaultRenderState;
    if (version <= LegacyMaterialFieldVersion)
    {
auto __delegateArg_00000087 = new Func<EngineBinaryReader*, Array<uint8_t>*>(static_cast<Array<uint8_t>* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadLegacyMaterialConstantBufferAsset));
auto __delegateArgDeleteGuard_00000088 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000087;
});
([&]() -> Array<Array<uint8_t>*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<Array<uint8_t>*>(__delegateArg_00000087);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<Array<uint8_t>*>(__delegateArg_00000087);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
    }
return materialAsset;}

::MaterialRenderState* EditorAssetBinarySerializer::ReadMaterialRenderState(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000089 = new ::MaterialRenderState();
__object_00000089->set_BlendMode(static_cast<MaterialBlendMode>(reader->ReadInt32()));
__object_00000089->set_CullMode(static_cast<MaterialCullMode>(reader->ReadInt32()));
__object_00000089->set_DepthTestEnabled(reader->ReadByte() != 0);
__object_00000089->set_DepthWriteEnabled(reader->ReadByte() != 0);
return __object_00000089;
})();}

::ModelAsset* EditorAssetBinarySerializer::ReadModelAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version < PreviousVersionWithoutRuntimeAssetId || version > CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
}
::ModelAsset *asset = new ::ModelAsset();
EditorAssetBinarySerializer::ReadAssetIdentity(reader, asset, static_cast<uint8_t>(version));
auto __delegateArg_0000008A = new Func<EngineBinaryReader*, float3>(static_cast<float3 (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadFloat3));
auto __delegateArgDeleteGuard_0000008B = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000008A;
});
asset->Positions = ([&]() -> Array<::float3>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<float3>(__delegateArg_0000008A);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<float3>(__delegateArg_0000008A);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
auto __delegateArg_0000008C = new Func<EngineBinaryReader*, float3>(static_cast<float3 (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadFloat3));
auto __delegateArgDeleteGuard_0000008D = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000008C;
});
asset->Normals = ([&]() -> Array<::float3>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<float3>(__delegateArg_0000008C);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<float3>(__delegateArg_0000008C);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
auto __delegateArg_0000008E = new Func<EngineBinaryReader*, float2>(static_cast<float2 (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadFloat2));
auto __delegateArgDeleteGuard_0000008F = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000008E;
});
asset->TexCoords = ([&]() -> Array<::float2>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<float2>(__delegateArg_0000008E);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<float2>(__delegateArg_0000008E);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
auto __delegateArg_00000090 = new Func<EngineBinaryReader*, uint16_t>(static_cast<uint16_t (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadUInt16Value));
auto __delegateArgDeleteGuard_00000091 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000090;
});
asset->Indices16 = ([&]() -> Array<uint16_t>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<uint16_t>(__delegateArg_00000090);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<uint16_t>(__delegateArg_00000090);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
auto __delegateArg_00000092 = new Func<EngineBinaryReader*, uint32_t>(static_cast<uint32_t (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadUInt32Value));
auto __delegateArgDeleteGuard_00000093 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000092;
});
asset->Indices32 = ([&]() -> Array<uint32_t>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<uint32_t>(__delegateArg_00000092);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<uint32_t>(__delegateArg_00000092);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
auto __delegateArg_00000094 = new Func<EngineBinaryReader*, ModelSubmeshAsset*>(static_cast<ModelSubmeshAsset* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadModelSubmeshAsset));
auto __delegateArgDeleteGuard_00000095 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000094;
});
asset->Submeshes = ([&]() -> Array<::ModelSubmeshAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<ModelSubmeshAsset*>(__delegateArg_00000094);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<ModelSubmeshAsset*>(__delegateArg_00000094);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
    if (version <= ModelPlatformPackedMeshTailVersion)
    {
reader->ReadByteArray();
    }
return asset;}

::ModelSubmeshAsset* EditorAssetBinarySerializer::ReadModelSubmeshAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_00000096 = new ::ModelSubmeshAsset();
__object_00000096->set_MaterialSlotName(reader->ReadString());
__object_00000096->set_IndexStart(reader->ReadInt32());
__object_00000096->set_IndexCount(reader->ReadInt32());
return __object_00000096;
})();}

::PlatformMaterialAsset* EditorAssetBinarySerializer::ReadPlatformMaterialAsset(::EngineBinaryReader* reader, uint8_t version)
{
::PlatformMaterialAsset *asset = new ::PlatformMaterialAsset();
EditorAssetBinarySerializer::ReadAssetIdentity(reader, asset, static_cast<uint8_t>(version));
asset->RendererFamilyId = reader->ReadString();
asset->TextureRelativePath = reader->ReadString();
asset->DoubleSided = reader->ReadByte() != 0;
asset->UseVertexColor = reader->ReadByte() != 0;
asset->Lit = reader->ReadByte() != 0;
asset->BaseColorR = reader->ReadByte();
asset->BaseColorG = reader->ReadByte();
asset->BaseColorB = reader->ReadByte();
asset->BaseColorA = reader->ReadByte();
return asset;}

::PlatformPositionKeyframeTrackAsset* EditorAssetBinarySerializer::ReadPlatformPositionKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_00000097 = new ::PlatformPositionKeyframeTrackAsset();
__object_00000097->set_Keyframes(([&]() {
auto __delegateArg_00000098 = new Func<EngineBinaryReader*, PositionKeyframeAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPositionKeyframeAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_00000099 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_00000098;
});
Array<::PositionKeyframeAsset*>* __coalesce_0000009A = ([&]() -> Array<::PositionKeyframeAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_00000098);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_00000098);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_0000009A != nullptr ? __coalesce_0000009A : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_00000097;
})();}

::PlatformRotationKeyframeTrackAsset* EditorAssetBinarySerializer::ReadPlatformRotationKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_0000009B = new ::PlatformRotationKeyframeTrackAsset();
__object_0000009B->set_Keyframes(([&]() {
auto __delegateArg_0000009C = new Func<EngineBinaryReader*, RotationKeyframeAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadRotationKeyframeAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_0000009D = he_cpp_make_scope_exit([&]() {
delete __delegateArg_0000009C;
});
Array<::RotationKeyframeAsset*>* __coalesce_0000009E = ([&]() -> Array<::RotationKeyframeAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<RotationKeyframeAsset*>(__delegateArg_0000009C);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<RotationKeyframeAsset*>(__delegateArg_0000009C);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_0000009E != nullptr ? __coalesce_0000009E : Array<RotationKeyframeAsset*>::Empty();
})());
return __object_0000009B;
})();}

::PositionKeyframeAsset* EditorAssetBinarySerializer::ReadPositionKeyframeAsset(::EngineBinaryReader* reader, uint8_t version)
{
::PositionKeyframeAsset *asset = new ::PositionKeyframeAsset();
    if (version >= AnimationClipPlatformOverrideVersion)
    {
asset->set_FrameId(reader->ReadString());
    }
asset->set_Time(reader->ReadSingle());
asset->set_Value(EditorAssetBinarySerializer::ReadFloat3(reader));
asset->set_InterpolationMode(EditorAssetBinarySerializer::ReadAnimationInterpolationMode(reader));
return asset;}

::PositionKeyframeTrackAsset* EditorAssetBinarySerializer::ReadPositionKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_0000009F = new ::PositionKeyframeTrackAsset();
__object_0000009F->set_Keyframes(([&]() {
auto __delegateArg_000000A0 = new Func<EngineBinaryReader*, PositionKeyframeAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPositionKeyframeAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_000000A1 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000A0;
});
Array<::PositionKeyframeAsset*>* __coalesce_000000A2 = ([&]() -> Array<::PositionKeyframeAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_000000A0);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_000000A0);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000A2 != nullptr ? __coalesce_000000A2 : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_0000009F;
})();}

::PositionOffsetKeyframeTrackAsset* EditorAssetBinarySerializer::ReadPositionOffsetKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_000000A3 = new ::PositionOffsetKeyframeTrackAsset();
__object_000000A3->set_Keyframes(([&]() {
auto __delegateArg_000000A4 = new Func<EngineBinaryReader*, PositionKeyframeAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPositionKeyframeAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_000000A5 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000A4;
});
Array<::PositionKeyframeAsset*>* __coalesce_000000A6 = ([&]() -> Array<::PositionKeyframeAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_000000A4);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_000000A4);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000A6 != nullptr ? __coalesce_000000A6 : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_000000A3;
})();}

::RotationKeyframeAsset* EditorAssetBinarySerializer::ReadRotationKeyframeAsset(::EngineBinaryReader* reader, uint8_t version)
{
::RotationKeyframeAsset *asset = new ::RotationKeyframeAsset();
    if (version >= AnimationClipPlatformOverrideVersion)
    {
asset->set_FrameId(reader->ReadString());
    }
asset->set_Time(reader->ReadSingle());
asset->set_Value(EditorAssetBinarySerializer::ReadFloat4(reader));
asset->set_InterpolationMode(EditorAssetBinarySerializer::ReadAnimationInterpolationMode(reader));
return asset;}

::RotationKeyframeTrackAsset* EditorAssetBinarySerializer::ReadRotationKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_000000A7 = new ::RotationKeyframeTrackAsset();
__object_000000A7->set_Keyframes(([&]() {
auto __delegateArg_000000A8 = new Func<EngineBinaryReader*, RotationKeyframeAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadRotationKeyframeAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_000000A9 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000A8;
});
Array<::RotationKeyframeAsset*>* __coalesce_000000AA = ([&]() -> Array<::RotationKeyframeAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<RotationKeyframeAsset*>(__delegateArg_000000A8);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<RotationKeyframeAsset*>(__delegateArg_000000A8);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000AA != nullptr ? __coalesce_000000AA : Array<RotationKeyframeAsset*>::Empty();
})());
return __object_000000A7;
})();}

::ScaleKeyframeTrackAsset* EditorAssetBinarySerializer::ReadScaleKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version)
{
return ([&]() {
auto __object_000000AB = new ::ScaleKeyframeTrackAsset();
__object_000000AB->set_Keyframes(([&]() {
auto __delegateArg_000000AC = new Func<EngineBinaryReader*, PositionKeyframeAsset*>([&](EngineBinaryReader* currentReader) {
return EditorAssetBinarySerializer::ReadPositionKeyframeAsset(currentReader, static_cast<uint8_t>(version));
});
auto __delegateArgDeleteGuard_000000AD = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000AC;
});
Array<::PositionKeyframeAsset*>* __coalesce_000000AE = ([&]() -> Array<::PositionKeyframeAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_000000AC);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<PositionKeyframeAsset*>(__delegateArg_000000AC);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000AE != nullptr ? __coalesce_000000AE : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_000000AB;
})();}

::SceneAsset* EditorAssetBinarySerializer::ReadSceneAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version < LegacyVersion || version > CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
}
::SceneAsset *asset = new ::SceneAsset();
EngineBinaryReadContext::set_CurrentReadStage("SceneAsset:Identity");
EditorAssetBinarySerializer::ReadAssetIdentity(reader, asset, static_cast<uint8_t>(version));
EngineBinaryReadContext::set_CurrentReadStage("SceneAsset:RootEntities");
asset->set_RootEntities(([&]() {
Array<::SceneEntityAsset*>* __coalesce_000000AF = EditorAssetBinarySerializer::ReadSceneEntityAssetArray(reader, static_cast<uint8_t>(version));
return __coalesce_000000AF != nullptr ? __coalesce_000000AF : Array<SceneEntityAsset*>::Empty();
})());
EngineBinaryReadContext::set_CurrentReadStage("SceneAsset:AssetReferences");
asset->set_AssetReferences(version >= 4 ? ([&]() {
Array<::SceneAssetReference*>* __coalesce_000000B0 = EditorAssetBinarySerializer::ReadSceneAssetReferenceArray(reader);
return __coalesce_000000B0 != nullptr ? __coalesce_000000B0 : Array<SceneAssetReference*>::Empty();
})() : Array<SceneAssetReference*>::Empty());
EngineBinaryReadContext::set_CurrentReadStage("SceneAsset:Physics3DSceneFeatureFlags");
asset->set_Physics3DSceneFeatureFlags(version >= 5 ? reader->ReadUInt32() : 0u);
EngineBinaryReadContext::set_CurrentReadStage("SceneAsset:SceneSettings");
asset->set_SceneSettings(version >= 6 ? EditorAssetBinarySerializer::ReadSceneSettingsAsset(reader, static_cast<uint8_t>(version)) : new ::SceneSettingsAsset());
return asset;}

::SceneAssetReference* EditorAssetBinarySerializer::ReadSceneAssetReference(::EngineBinaryReader* reader)
{
return SceneAssetReferenceFactory::ReadRequiredReference(reader);}

Array<::SceneAssetReference*>* EditorAssetBinarySerializer::ReadSceneAssetReferenceArray(::EngineBinaryReader* reader)
{
EngineBinaryReadContext::set_CurrentReadStage("SceneAssetReferenceArray:Length");
auto __delegateArg_000000B1 = new Func<EngineBinaryReader*, SceneAssetReference*>(static_cast<SceneAssetReference* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadSceneAssetReference));
auto __delegateArgDeleteGuard_000000B2 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000B1;
});
return ([&]() -> Array<::SceneAssetReference*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneAssetReference*>(__delegateArg_000000B1);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneAssetReference*>(__delegateArg_000000B1);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();}

::SceneCanvasProfile* EditorAssetBinarySerializer::ReadSceneCanvasProfile(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_000000B3 = new ::SceneCanvasProfile();
__object_000000B3->set_Width(reader->ReadInt32());
__object_000000B3->set_Height(reader->ReadInt32());
return __object_000000B3;
})();}

::SceneComponentAssetRecord* EditorAssetBinarySerializer::ReadSceneComponentAssetRecord(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
EngineBinaryReadContext::set_CurrentReadStage("SceneComponentRecord:ComponentKey");
const std::string componentKey = sceneEntityPayloadVersion >= 3 ? reader->ReadString() : String::Empty;
EngineBinaryReadContext::set_CurrentReadStage("SceneComponentRecord:ComponentTypeId");
const std::string componentTypeId = reader->ReadString();
EngineBinaryReadContext::set_CurrentReadStage("SceneComponentRecord:ComponentIndex");
const int32_t componentIndex = reader->ReadInt32();
EngineBinaryReadContext::set_CurrentReadStage(std::string("SceneComponentRecord:Payload:") + componentTypeId);
return ([&]() {
auto __object_000000B4 = new ::SceneComponentAssetRecord();
__object_000000B4->set_ComponentKey(componentKey);
__object_000000B4->set_ComponentTypeId(componentTypeId);
__object_000000B4->set_ComponentIndex(componentIndex);
__object_000000B4->set_Payload(([&]() {
Array<uint8_t>* __coalesce_000000B5 = reader->ReadByteArray();
return __coalesce_000000B5 != nullptr ? __coalesce_000000B5 : Array<uint8_t>::Empty();
})());
return __object_000000B4;
})();}

Array<::SceneComponentAssetRecord*>* EditorAssetBinarySerializer::ReadSceneComponentAssetRecordArray(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion)
{
EngineBinaryReadContext::set_CurrentReadStage("SceneComponentRecordArray:Length");
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else {
    if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
else {
    if (length == 0)
    {
return Array<SceneComponentAssetRecord*>::Empty();    }
}
}
Array<::SceneComponentAssetRecord*> *values = new Array<SceneComponentAssetRecord*>(length);
for (int32_t index = 0; index < values->get_Length(); index++) {
EngineBinaryReadContext::set_CurrentReadStage(std::string("SceneComponentRecordArray:Element:") + std::to_string(index));
(*values)[index] = EditorAssetBinarySerializer::ReadSceneComponentAssetRecord(reader, static_cast<uint8_t>(sceneEntityPayloadVersion));
}
return values;}

::SceneEntityAsset* EditorAssetBinarySerializer::ReadSceneEntityAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version < LegacyVersion || version > CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
}
    if (version == LegacyVersion)
    {
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:LegacyPayload");
return EditorAssetBinarySerializer::ReadLegacySceneEntityAsset(reader);    }
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:PayloadVersion");
const uint8_t payloadVersion = reader->ReadByte();
    if (payloadVersion != 1 && payloadVersion != 2 && payloadVersion != 3 && payloadVersion != 4 && payloadVersion != 5 && payloadVersion != SceneEntityPayloadVersion)
    {
throw new InvalidOperationException(std::string("Unsupported scene entity payload version '") + std::to_string(payloadVersion) + std::string("'."));
    }
uint32_t id = 0u;
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:Identity");
    if (payloadVersion >= 4)
    {
id = reader->ReadUInt32();
    }
else {
reader->ReadString();
}
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:Name");
const std::string name = reader->ReadString();
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:Transform");
const bool isStatic = payloadVersion >= 4 && reader->ReadByte() != 0;
const bool enabled = payloadVersion >= 6 ? reader->ReadByte() != 0 : true;
const uint16_t layerMask = payloadVersion >= 5 ? reader->ReadUInt16() : static_cast<uint16_t>(0b00000001);
::float3 localPosition = reader->ReadFloat3();
::float3 localScale = reader->ReadFloat3();
::float4 localOrientation = reader->ReadFloat4();
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:Components");
Array<::SceneComponentAssetRecord*> *components = ([&]() {
Array<::SceneComponentAssetRecord*>* __coalesce_000000B6 = EditorAssetBinarySerializer::ReadSceneComponentAssetRecordArray(reader, static_cast<uint8_t>(payloadVersion));
return __coalesce_000000B6 != nullptr ? __coalesce_000000B6 : Array<SceneComponentAssetRecord*>::Empty();
})();
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:PlatformExistenceOverrides");
Array<::SceneEntityPlatformExistenceOverrideAsset*> *platformExistenceOverrides = payloadVersion >= 7 ? ([&]() {
auto __delegateArg_000000B7 = new Func<EngineBinaryReader*, SceneEntityPlatformExistenceOverrideAsset*>(static_cast<SceneEntityPlatformExistenceOverrideAsset* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformExistenceOverrideAsset));
auto __delegateArgDeleteGuard_000000B8 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000B7;
});
Array<::SceneEntityPlatformExistenceOverrideAsset*>* __coalesce_000000B9 = ([&]() -> Array<::SceneEntityPlatformExistenceOverrideAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformExistenceOverrideAsset*>(__delegateArg_000000B7);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformExistenceOverrideAsset*>(__delegateArg_000000B7);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000B9 != nullptr ? __coalesce_000000B9 : Array<SceneEntityPlatformExistenceOverrideAsset*>::Empty();
})() : Array<SceneEntityPlatformExistenceOverrideAsset*>::Empty();
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:PlatformTransformOverrides");
Array<::SceneEntityPlatformTransformOverrideAsset*> *platformTransformOverrides = payloadVersion >= 2 ? ([&]() {
auto __delegateArg_000000BA = new Func<EngineBinaryReader*, SceneEntityPlatformTransformOverrideAsset*>(static_cast<SceneEntityPlatformTransformOverrideAsset* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformTransformOverrideAsset));
auto __delegateArgDeleteGuard_000000BB = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000BA;
});
Array<::SceneEntityPlatformTransformOverrideAsset*>* __coalesce_000000BC = ([&]() -> Array<::SceneEntityPlatformTransformOverrideAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformTransformOverrideAsset*>(__delegateArg_000000BA);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformTransformOverrideAsset*>(__delegateArg_000000BA);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000BC != nullptr ? __coalesce_000000BC : Array<SceneEntityPlatformTransformOverrideAsset*>::Empty();
})() : Array<SceneEntityPlatformTransformOverrideAsset*>::Empty();
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:PlatformComponentOverrides");
Array<::SceneEntityPlatformComponentOverrideAsset*> *platformComponentOverrides = payloadVersion >= 3 ? ([&]() {
auto __delegateArg_000000BD = new Func<EngineBinaryReader*, SceneEntityPlatformComponentOverrideAsset*>(static_cast<SceneEntityPlatformComponentOverrideAsset* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformComponentOverrideAsset));
auto __delegateArgDeleteGuard_000000BE = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000BD;
});
Array<::SceneEntityPlatformComponentOverrideAsset*>* __coalesce_000000BF = ([&]() -> Array<::SceneEntityPlatformComponentOverrideAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformComponentOverrideAsset*>(__delegateArg_000000BD);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformComponentOverrideAsset*>(__delegateArg_000000BD);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000BF != nullptr ? __coalesce_000000BF : Array<SceneEntityPlatformComponentOverrideAsset*>::Empty();
})() : Array<SceneEntityPlatformComponentOverrideAsset*>::Empty();
EngineBinaryReadContext::set_CurrentReadStage("SceneEntity:Children");
Array<::SceneEntityAsset*> *children = ([&]() {
Array<::SceneEntityAsset*>* __coalesce_000000C0 = EditorAssetBinarySerializer::ReadSceneEntityAssetArray(reader, static_cast<uint8_t>(version));
return __coalesce_000000C0 != nullptr ? __coalesce_000000C0 : Array<SceneEntityAsset*>::Empty();
})();
EngineBinaryReadContext::set_LastCheckpoint(std::string("SceneEntityEnd:") + name + std::string("@") + std::to_string(reader->GetStreamPosition()));
return ([&]() {
auto __object_000000C1 = new ::SceneEntityAsset();
__object_000000C1->set_Id(id);
__object_000000C1->set_Name(name);
__object_000000C1->set_IsStatic(isStatic);
__object_000000C1->set_Enabled(enabled);
__object_000000C1->set_LayerMask(layerMask);
__object_000000C1->set_LocalPosition(localPosition);
__object_000000C1->set_LocalScale(localScale);
__object_000000C1->set_LocalOrientation(localOrientation);
__object_000000C1->set_Components(components);
__object_000000C1->set_PlatformExistenceOverrides(platformExistenceOverrides);
__object_000000C1->set_PlatformTransformOverrides(platformTransformOverrides);
__object_000000C1->set_PlatformComponentOverrides(platformComponentOverrides);
__object_000000C1->set_Children(children);
return __object_000000C1;
})();}

Array<::SceneEntityAsset*>* EditorAssetBinarySerializer::ReadSceneEntityAssetArray(::EngineBinaryReader* reader, uint8_t version)
{
EngineBinaryReadContext::set_CurrentReadStage("SceneEntityArray:Length");
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else {
    if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
else {
    if (length == 0)
    {
return Array<SceneEntityAsset*>::Empty();    }
}
}
Array<::SceneEntityAsset*> *values = new Array<SceneEntityAsset*>(length);
for (int32_t index = 0; index < values->get_Length(); index++) {
EngineBinaryReadContext::set_CurrentReadStage(std::string("SceneEntityArray:Element:") + std::to_string(index));
(*values)[index] = EditorAssetBinarySerializer::ReadSceneEntityAsset(reader, static_cast<uint8_t>(version));
}
return values;}

::SceneEntityPlatformAddedComponentAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAsset(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_000000C2 = new ::SceneEntityPlatformAddedComponentAsset();
__object_000000C2->set_Component(EditorAssetBinarySerializer::ReadSceneComponentAssetRecord(reader, static_cast<uint8_t>(sceneEntityPayloadVersion)));
return __object_000000C2;
})();}

::SceneEntityPlatformAddedComponentAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAssetValue(::EngineBinaryReader* reader)
{
return EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAsset(reader, static_cast<uint8_t>(SceneEntityPayloadVersion));}

::SceneEntityPlatformComponentOverrideAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformComponentOverrideAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_000000C3 = new ::SceneEntityPlatformComponentOverrideAsset();
__object_000000C3->set_PlatformId(reader->ReadString());
__object_000000C3->set_RemovedComponentKeys(([&]() {
auto __delegateArg_000000C4 = new Func<EngineBinaryReader*, std::string>(static_cast<std::string (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadStringValue));
auto __delegateArgDeleteGuard_000000C5 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000C4;
});
Array<std::string>* __coalesce_000000C6 = ([&]() -> Array<std::string>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<std::string>(__delegateArg_000000C4);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<std::string>(__delegateArg_000000C4);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000C6 != nullptr ? __coalesce_000000C6 : Array<std::string>::Empty();
})());
__object_000000C3->set_AddedComponents(([&]() {
auto __delegateArg_000000C7 = new Func<EngineBinaryReader*, SceneEntityPlatformAddedComponentAsset*>(static_cast<SceneEntityPlatformAddedComponentAsset* (*)(EngineBinaryReader*)>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAssetValue));
auto __delegateArgDeleteGuard_000000C8 = he_cpp_make_scope_exit([&]() {
delete __delegateArg_000000C7;
});
Array<::SceneEntityPlatformAddedComponentAsset*>* __coalesce_000000C9 = ([&]() -> Array<::SceneEntityPlatformAddedComponentAsset*>* {
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderLE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformAddedComponentAsset*>(__delegateArg_000000C7);
}
if (auto heCppDispatchImpl = dynamic_cast<::BinaryReaderBE*>(reader)) {
return heCppDispatchImpl->ReadArray<SceneEntityPlatformAddedComponentAsset*>(__delegateArg_000000C7);
}
throw new NotSupportedException("No generated implementation matched generic dispatch receiver.");
})();
return __coalesce_000000C9 != nullptr ? __coalesce_000000C9 : Array<SceneEntityPlatformAddedComponentAsset*>::Empty();
})());
return __object_000000C3;
})();}

::SceneEntityPlatformExistenceOverrideAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformExistenceOverrideAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_000000CA = new ::SceneEntityPlatformExistenceOverrideAsset();
__object_000000CA->set_PlatformId(reader->ReadString());
__object_000000CA->set_Exists(reader->ReadByte() != 0);
return __object_000000CA;
})();}

::SceneEntityPlatformTransformOverrideAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformTransformOverrideAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_000000CB = new ::SceneEntityPlatformTransformOverrideAsset();
__object_000000CB->set_PlatformId(reader->ReadString());
__object_000000CB->set_HasLocalPositionOverride(reader->ReadByte() != 0);
__object_000000CB->set_LocalPosition(reader->ReadFloat3());
__object_000000CB->set_HasLocalScaleOverride(reader->ReadByte() != 0);
__object_000000CB->set_LocalScale(reader->ReadFloat3());
__object_000000CB->set_HasLocalOrientationOverride(reader->ReadByte() != 0);
__object_000000CB->set_LocalOrientation(reader->ReadFloat4());
return __object_000000CB;
})();}

::SceneSettingsAsset* EditorAssetBinarySerializer::ReadSceneSettingsAsset(::EngineBinaryReader* reader, uint8_t version)
{
::SceneSettingsAsset *sceneSettings = ([&]() {
auto __object_000000CC = new ::SceneSettingsAsset();
__object_000000CC->set_CanvasProfile(EditorAssetBinarySerializer::ReadSceneCanvasProfile(reader));
return __object_000000CC;
})();
    if (version >= 15)
    {
sceneSettings->set_DontUnload(EditorAssetBinarySerializer::ReadBooleanByte(reader, "scene settings"));
    }
return sceneSettings;}

std::string EditorAssetBinarySerializer::ReadStringValue(::EngineBinaryReader* reader)
{
return reader->ReadString();}

::TextAsset* EditorAssetBinarySerializer::ReadTextAsset(::EngineBinaryReader* reader, uint8_t version)
{
::TextAsset *asset = new ::TextAsset();
EditorAssetBinarySerializer::ReadAssetIdentity(reader, asset, static_cast<uint8_t>(version));
asset->Text = reader->ReadString();
return asset;}

::TextureAsset* EditorAssetBinarySerializer::ReadTextureAsset(::EngineBinaryReader* reader, uint8_t version)
{
::TextureAsset *asset = new ::TextureAsset();
EditorAssetBinarySerializer::ReadAssetIdentity(reader, asset, static_cast<uint8_t>(version));
asset->Width = reader->ReadUInt16();
asset->Height = reader->ReadUInt16();
asset->ColorFormat = version >= TextureColorFormatVersion ? EditorAssetBinarySerializer::ReadTextureAssetColorFormat(reader) : TextureAssetColorFormat::Rgba32;
asset->AlphaPrecision = version >= TexturePaletteMetadataVersion ? EditorAssetBinarySerializer::ReadTextureAssetAlphaPrecision(reader) : EditorAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(static_cast<TextureAssetColorFormat>(asset->ColorFormat));
asset->PaletteColors = version >= TexturePaletteMetadataVersion ? reader->ReadByteArray() : Array<uint8_t>::Empty();
asset->Colors = reader->ReadByteArray();
return asset;}

::TextureAssetAlphaPrecision EditorAssetBinarySerializer::ReadTextureAssetAlphaPrecision(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
const uint8_t serializedValue = reader->ReadByte();
    if (serializedValue == static_cast<uint8_t>(TextureAssetAlphaPrecision::Opaque))
    {
return TextureAssetAlphaPrecision::Opaque;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetAlphaPrecision::Binary))
    {
return TextureAssetAlphaPrecision::Binary;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetAlphaPrecision::A4))
    {
return TextureAssetAlphaPrecision::A4;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetAlphaPrecision::A8))
    {
return TextureAssetAlphaPrecision::A8;    }
}
}
}
throw new InvalidOperationException(std::string("Unsupported texture alpha precision '") + std::to_string(serializedValue) + std::string("'."));
}

::TextureAssetColorFormat EditorAssetBinarySerializer::ReadTextureAssetColorFormat(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
const uint8_t serializedValue = reader->ReadByte();
    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Rgba32))
    {
return TextureAssetColorFormat::Rgba32;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Rgba4444))
    {
return TextureAssetColorFormat::Rgba4444;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Indexed4))
    {
return TextureAssetColorFormat::Indexed4;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Indexed8))
    {
return TextureAssetColorFormat::Indexed8;    }
else {
    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::GxRgb5A3))
    {
return TextureAssetColorFormat::GxRgb5A3;    }
}
}
}
}
throw new InvalidOperationException(std::string("Unsupported texture color format '") + std::to_string(serializedValue) + std::string("'."));
}

uint16_t EditorAssetBinarySerializer::ReadUInt16Value(::EngineBinaryReader* reader)
{
return reader->ReadUInt16();}

uint32_t EditorAssetBinarySerializer::ReadUInt32Value(::EngineBinaryReader* reader)
{
return reader->ReadUInt32();}

void EditorAssetBinarySerializer::ValidateHeader(::EngineBinaryHeader* header)
{
    if (header->FormatId != FormatId)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary format id '") + std::to_string(header->FormatId) + std::string("'."));
    }
else {
    if (header->RecordKind != static_cast<uint16_t>(RecordKind))
    {
throw new InvalidOperationException(std::string("Unexpected asset record kind '") + std::to_string(header->RecordKind) + std::string("'."));
    }
else {
    if (header->Version < LegacyVersion || header->Version > CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(header->Version) + std::string("'."));
    }
}
}
}

