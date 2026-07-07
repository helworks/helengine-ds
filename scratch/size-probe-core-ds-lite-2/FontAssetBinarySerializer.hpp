#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class FontAsset;
class EngineBinaryHeader;
class EngineBinaryReader;

#include "EditorBinaryRecordKind.hpp"
#include "runtime/native_string.hpp"
#include "system/io/stream.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "TextureAssetColorFormat.hpp"

class FontAssetBinarySerializer
{
public:
    virtual ~FontAssetBinarySerializer() = default;

    inline static const uint16_t FormatId = 1;

    static const ::EditorBinaryRecordKind RecordKind;

    inline static const uint8_t CurrentVersion = 5;

    static std::string LastDeserializeStage;

    static const std::string& get_LastDeserializeStage();
    static void set_LastDeserializeStage(std::string value);

    static ::FontAsset* Deserialize(::Stream* stream);

    static ::FontAsset* Deserialize(::Stream* stream, ::EngineBinaryHeader* header);
private:
    inline static const uint8_t RuntimeTextureIdVersion = 2;

    inline static const uint8_t TextureColorFormatVersion = 3;

    inline static const uint8_t PaletteTextureMetadataVersion = 4;

    inline static const uint8_t ExternalCookedAtlasPathVersion = 5;

    static ::TextureAssetAlphaPrecision GetDefaultTextureAssetAlphaPrecision(::TextureAssetColorFormat colorFormat);

    static ::TextureAssetAlphaPrecision ReadTextureAssetAlphaPrecision(::EngineBinaryReader* reader);

    static ::TextureAssetColorFormat ReadTextureAssetColorFormat(::EngineBinaryReader* reader);

    static void ValidateHeader(::EngineBinaryHeader* header);
};
