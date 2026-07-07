#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class EngineBinaryHeader;
class SceneAsset;
class AnimationClipAsset;
class EngineBinaryReader;
class AnimationClipPlatformOverrideAsset;
class float2;
class float3;
class float4;
class SceneComponentAssetRecord;
class SceneEntityAsset;
class MaterialAsset;
class MaterialRenderState;
class ModelAsset;
class ModelSubmeshAsset;
class PlatformMaterialAsset;
class PlatformPositionKeyframeTrackAsset;
class PlatformRotationKeyframeTrackAsset;
class PositionKeyframeAsset;
class PositionKeyframeTrackAsset;
class PositionOffsetKeyframeTrackAsset;
class RotationKeyframeAsset;
class RotationKeyframeTrackAsset;
class ScaleKeyframeTrackAsset;
class SceneAssetReference;
class SceneCanvasProfile;
class SceneEntityPlatformAddedComponentAsset;
class SceneEntityPlatformComponentOverrideAsset;
class SceneEntityPlatformExistenceOverrideAsset;
class SceneEntityPlatformTransformOverrideAsset;
class SceneSettingsAsset;
class TextAsset;
class TextureAsset;

#include "EditorBinaryRecordKind.hpp"
#include "system/io/stream.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "TextureAssetColorFormat.hpp"
#include "AnimationInterpolationMode.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "runtime/native_string.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class EditorAssetBinarySerializer
{
public:
    virtual ~EditorAssetBinarySerializer() = default;

    inline static const uint16_t FormatId = 1;

    static const ::EditorBinaryRecordKind RecordKind;

    inline static const uint8_t CurrentVersion = 19;

    static ::Asset* Deserialize(::Stream* stream);

    static ::Asset* Deserialize(::Stream* stream, ::EngineBinaryHeader* header);

    static ::SceneAsset* DeserializeSceneAsset(::Stream* stream);

    static ::SceneAsset* DeserializeSceneAsset(::Stream* stream, ::EngineBinaryHeader* header);
private:
    inline static const uint8_t LegacyVersion = 2;

    inline static const uint8_t PreviousVersionWithoutRuntimeAssetId = 10;

    inline static const uint8_t TextureColorFormatVersion = 13;

    inline static const uint8_t TexturePaletteMetadataVersion = 14;

    inline static const uint8_t ModelPlatformPackedMeshTailVersion = 15;

    inline static const uint8_t LegacyMaterialFieldVersion = 16;

    inline static const uint8_t SceneEntityPayloadVersion = 7;

    inline static const uint8_t AnimationClipPlatformOverrideVersion = 19;

    static ::TextureAssetAlphaPrecision GetDefaultTextureAssetAlphaPrecision(::TextureAssetColorFormat colorFormat);

    static ::AnimationClipAsset* ReadAnimationClipAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::AnimationClipPlatformOverrideAsset* ReadAnimationClipPlatformOverrideAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::AnimationInterpolationMode ReadAnimationInterpolationMode(::EngineBinaryReader* reader);

    static void ReadAssetIdentity(::EngineBinaryReader* reader, ::Asset* asset, uint8_t version);

    static ::Asset* ReadAssetPayload(::EngineBinaryReader* reader, ::EditorAssetBinaryValueKind valueKind, uint8_t version);

    static bool ReadBooleanByte(::EngineBinaryReader* reader, std::string context);

    static ::float2 ReadFloat2(::EngineBinaryReader* reader);

    static ::float3 ReadFloat3(::EngineBinaryReader* reader);

    static ::float4 ReadFloat4(::EngineBinaryReader* reader);

    static Array<uint8_t>* ReadLegacyMaterialConstantBufferAsset(::EngineBinaryReader* reader);

    static ::SceneComponentAssetRecord* ReadLegacySceneComponentAssetRecord(::EngineBinaryReader* reader);

    static ::SceneEntityAsset* ReadLegacySceneEntityAsset(::EngineBinaryReader* reader);

    static Array<::SceneEntityAsset*>* ReadLegacySceneEntityAssetArray(::EngineBinaryReader* reader);

    static ::MaterialAsset* ReadMaterialAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::MaterialRenderState* ReadMaterialRenderState(::EngineBinaryReader* reader);

    static ::ModelAsset* ReadModelAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::ModelSubmeshAsset* ReadModelSubmeshAsset(::EngineBinaryReader* reader);

    static ::PlatformMaterialAsset* ReadPlatformMaterialAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::PlatformPositionKeyframeTrackAsset* ReadPlatformPositionKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::PlatformRotationKeyframeTrackAsset* ReadPlatformRotationKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::PositionKeyframeAsset* ReadPositionKeyframeAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::PositionKeyframeTrackAsset* ReadPositionKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::PositionOffsetKeyframeTrackAsset* ReadPositionOffsetKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::RotationKeyframeAsset* ReadRotationKeyframeAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::RotationKeyframeTrackAsset* ReadRotationKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::ScaleKeyframeTrackAsset* ReadScaleKeyframeTrackAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::SceneAsset* ReadSceneAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::SceneAssetReference* ReadSceneAssetReference(::EngineBinaryReader* reader);

    static Array<::SceneAssetReference*>* ReadSceneAssetReferenceArray(::EngineBinaryReader* reader);

    static ::SceneCanvasProfile* ReadSceneCanvasProfile(::EngineBinaryReader* reader);

    static ::SceneComponentAssetRecord* ReadSceneComponentAssetRecord(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion);

    static Array<::SceneComponentAssetRecord*>* ReadSceneComponentAssetRecordArray(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion);

    static ::SceneEntityAsset* ReadSceneEntityAsset(::EngineBinaryReader* reader, uint8_t version);

    static Array<::SceneEntityAsset*>* ReadSceneEntityAssetArray(::EngineBinaryReader* reader, uint8_t version);

    static ::SceneEntityPlatformAddedComponentAsset* ReadSceneEntityPlatformAddedComponentAsset(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion);

    static ::SceneEntityPlatformAddedComponentAsset* ReadSceneEntityPlatformAddedComponentAssetValue(::EngineBinaryReader* reader);

    static ::SceneEntityPlatformComponentOverrideAsset* ReadSceneEntityPlatformComponentOverrideAsset(::EngineBinaryReader* reader);

    static ::SceneEntityPlatformExistenceOverrideAsset* ReadSceneEntityPlatformExistenceOverrideAsset(::EngineBinaryReader* reader);

    static ::SceneEntityPlatformTransformOverrideAsset* ReadSceneEntityPlatformTransformOverrideAsset(::EngineBinaryReader* reader);

    static ::SceneSettingsAsset* ReadSceneSettingsAsset(::EngineBinaryReader* reader, uint8_t version);

    static std::string ReadStringValue(::EngineBinaryReader* reader);

    static ::TextAsset* ReadTextAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::TextureAsset* ReadTextureAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::TextureAssetAlphaPrecision ReadTextureAssetAlphaPrecision(::EngineBinaryReader* reader);

    static ::TextureAssetColorFormat ReadTextureAssetColorFormat(::EngineBinaryReader* reader);

    static uint16_t ReadUInt16Value(::EngineBinaryReader* reader);

    static uint32_t ReadUInt32Value(::EngineBinaryReader* reader);

    static void ValidateHeader(::EngineBinaryHeader* header);
};
