#ifdef DrawText
#undef DrawText
#endif
#include "FontAssetBinarySerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "FontAsset.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_string.hpp"
#include "TextureAsset.hpp"
#include "FontInfo.hpp"
#include "runtime/native_dictionary.hpp"
#include "RuntimeTexture.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "FontAssetBinarySerializer.hpp"
#include "Core.hpp"
#include "FontChar.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryEndianness.hpp"
#include "float2.hpp"
#include "FontTightMetrics.hpp"
#include "runtime/array.hpp"
#include "system/func.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "SceneEntityReference.hpp"
#include "runtime/native_span.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "RenderManager2D.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "NativeOwnership.hpp"
#include "Asset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_span.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/io/stream.hpp"

const ::EditorBinaryRecordKind FontAssetBinarySerializer::RecordKind = EditorBinaryRecordKind::FontAsset;

std::string FontAssetBinarySerializer::LastDeserializeStage = String::Empty;

const std::string& FontAssetBinarySerializer::get_LastDeserializeStage()
{
return FontAssetBinarySerializer::LastDeserializeStage;
}

void FontAssetBinarySerializer::set_LastDeserializeStage(std::string value)
{
FontAssetBinarySerializer::LastDeserializeStage = value;
}

::FontAsset* FontAssetBinarySerializer::Deserialize(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
{
auto __finallyGuard_000000E0 = he_cpp_make_scope_exit([&]() {
delete header;
});
return FontAssetBinarySerializer::Deserialize(stream, header);}
}

::FontAsset* FontAssetBinarySerializer::Deserialize(::Stream* stream, ::EngineBinaryHeader* header)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
FontAssetBinarySerializer::ValidateHeader(header);
    if (Core::Instance == nullptr || Core::Instance->RenderManager2D == nullptr)
    {
throw new InvalidOperationException("Font assets require an initialized core renderer before deserialization.");
    }
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, static_cast<EngineBinaryEndianness>(header->Endianness), true);
auto __usingDisposeGuard_000000E1 = he_cpp_make_scope_exit([&]() {
if (reader != nullptr) {
reader->Dispose();
delete reader;
}
});
std::string cookedAtlasTextureRelativePath = String::Empty;
::TextureAsset *sourceTexture = new ::TextureAsset();
::FontInfo *fontInfo;
float lineHeight;
int32_t atlasWidth;
int32_t atlasHeight;
    if (header->Version >= ExternalCookedAtlasPathVersion)
    {
cookedAtlasTextureRelativePath = reader->ReadString();
sourceTexture->set_RuntimeAssetId(header->Version >= RuntimeTextureIdVersion ? static_cast<uint64_t>(reader->ReadInt64()) : 0ul);
sourceTexture->Width = reader->ReadUInt16();
sourceTexture->Height = reader->ReadUInt16();
sourceTexture->ColorFormat = header->Version >= TextureColorFormatVersion ? FontAssetBinarySerializer::ReadTextureAssetColorFormat(reader) : TextureAssetColorFormat::Rgba32;
sourceTexture->AlphaPrecision = header->Version >= PaletteTextureMetadataVersion ? FontAssetBinarySerializer::ReadTextureAssetAlphaPrecision(reader) : FontAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(static_cast<TextureAssetColorFormat>(sourceTexture->ColorFormat));
sourceTexture->PaletteColors = header->Version >= PaletteTextureMetadataVersion ? reader->ReadByteArray() : Array<uint8_t>::Empty();
sourceTexture->Colors = reader->ReadByteArray();
fontInfo = ([&]() {
auto __ctor_arg_000000E2 = reader->ReadString();
auto __ctor_arg_000000E3 = static_cast<int32_t>(reader->ReadInt32());
auto __ctor_arg_000000E4 = reader->ReadSingle();
return new ::FontInfo(__ctor_arg_000000E2, __ctor_arg_000000E3, __ctor_arg_000000E4);
})();
lineHeight = reader->ReadSingle();
atlasWidth = reader->ReadInt32();
atlasHeight = reader->ReadInt32();
    }
else {
fontInfo = ([&]() {
auto __ctor_arg_000000E5 = reader->ReadString();
auto __ctor_arg_000000E6 = static_cast<int32_t>(reader->ReadInt32());
auto __ctor_arg_000000E7 = reader->ReadSingle();
return new ::FontInfo(__ctor_arg_000000E5, __ctor_arg_000000E6, __ctor_arg_000000E7);
})();
lineHeight = reader->ReadSingle();
atlasWidth = reader->ReadInt32();
atlasHeight = reader->ReadInt32();
sourceTexture->set_RuntimeAssetId(header->Version >= RuntimeTextureIdVersion ? static_cast<uint64_t>(reader->ReadInt64()) : 0ul);
sourceTexture->Width = reader->ReadUInt16();
sourceTexture->Height = reader->ReadUInt16();
sourceTexture->ColorFormat = header->Version >= TextureColorFormatVersion ? FontAssetBinarySerializer::ReadTextureAssetColorFormat(reader) : TextureAssetColorFormat::Rgba32;
sourceTexture->AlphaPrecision = header->Version >= PaletteTextureMetadataVersion ? FontAssetBinarySerializer::ReadTextureAssetAlphaPrecision(reader) : FontAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(static_cast<TextureAssetColorFormat>(sourceTexture->ColorFormat));
sourceTexture->PaletteColors = header->Version >= PaletteTextureMetadataVersion ? reader->ReadByteArray() : Array<uint8_t>::Empty();
sourceTexture->Colors = reader->ReadByteArray();
}
const int32_t characterCount = reader->ReadInt32();
Dictionary<char, ::FontChar> *characters = new Dictionary<char, ::FontChar>(static_cast<int32_t>(characterCount));
for (int32_t index = 0; index < characterCount; index++) {
const char character = static_cast<char>(reader->ReadUInt16());
::FontChar fontChar = ([&]() {
auto __ctor_arg_000000E8 = reader->ReadFloat4();
auto __ctor_arg_000000E9 = reader->ReadSingle();
auto __ctor_arg_000000EA = reader->ReadSingle();
auto __ctor_arg_000000EB = reader->ReadSingle();
auto __ctor_arg_000000EC = reader->ReadSingle();
return ::FontChar(__ctor_arg_000000E8, __ctor_arg_000000E9, __ctor_arg_000000EA, __ctor_arg_000000EB, __ctor_arg_000000EC);
})();
characters->Add(static_cast<char>(character), fontChar);
}
::RuntimeTexture *texture = nullptr;
::TextureAsset *storedSourceTextureAsset = nullptr;
    if (sourceTexture->Width > 0 && sourceTexture->Height > 0 && sourceTexture->Colors != nullptr && sourceTexture->Colors->get_Length() > 0)
    {
texture = Core::Instance->RenderManager2D->BuildTextureFromRaw(sourceTexture);
storedSourceTextureAsset = sourceTexture;
    }
::FontAsset *asset = ([&]() {
auto __object_000000ED = new ::FontAsset(fontInfo, texture, characters, lineHeight, static_cast<int32_t>(atlasWidth), static_cast<int32_t>(atlasHeight));
__object_000000ED->set_SourceTextureAsset(storedSourceTextureAsset);
__object_000000ED->set_CookedAtlasTextureRelativePath(cookedAtlasTextureRelativePath);
return __object_000000ED;
})();
return asset;}
}

::TextureAssetAlphaPrecision FontAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(::TextureAssetColorFormat colorFormat)
{
    if (colorFormat == TextureAssetColorFormat::Rgba4444)
    {
return TextureAssetAlphaPrecision::A4;    }
return TextureAssetAlphaPrecision::A8;}

::TextureAssetAlphaPrecision FontAssetBinarySerializer::ReadTextureAssetAlphaPrecision(::EngineBinaryReader* reader)
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

::TextureAssetColorFormat FontAssetBinarySerializer::ReadTextureAssetColorFormat(::EngineBinaryReader* reader)
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

void FontAssetBinarySerializer::ValidateHeader(::EngineBinaryHeader* header)
{
    if (header->FormatId != FormatId)
    {
throw new InvalidOperationException(std::string("Unsupported font binary format id '") + std::to_string(header->FormatId) + std::string("'."));
    }
    if (header->RecordKind != static_cast<uint16_t>(RecordKind))
    {
throw new InvalidOperationException(std::string("Unexpected font record kind '") + std::to_string(header->RecordKind) + std::string("'."));
    }
    if (header->Version < 1 || header->Version > CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported font binary version '") + std::to_string(header->Version) + std::string("'."));
    }
}

