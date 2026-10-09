#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "NintendoDsTextureCodec.hpp"
namespace helengine::ds {
    /// Creates one DS runtime texture with no prepared hardware payload yet.
    NintendoDsRuntimeTexture2D::NintendoDsRuntimeTexture2D()
        : NativeAsset(nullptr)
        , ColorFormat(TextureAssetColorFormat::Rgba32)
        , AlphaPrecision(TextureAssetAlphaPrecision::A8)
        , Colors(Array<uint8_t>::Empty())
        , PaletteColors(Array<uint8_t>::Empty())
        , HardwareTextureId(-1)
        , HardwareTextureUploaded(false)
        , MainHardwareSpriteGraphics()
        , MainHardwareSpritePrepared(false)
        , MainHardwareSpriteUses256Color(false)
        , MainHardwareSpritePaletteBank(-1)
        , SubHardwareSpriteGraphics()
        , SubHardwareSpritePrepared(false)
        , SubHardwareSpriteUses256Color(false)
        , SubHardwareSpritePaletteBank(-1)
        , MainHardwareSpriteTileCount(0)
        , SubHardwareSpriteTileCount(0) {
    }

    /// Releases renderer-owned pixel and palette payloads without deleting Array<T>::Empty().
    NintendoDsRuntimeTexture2D::~NintendoDsRuntimeTexture2D() {
        Array<uint8_t>* colors = Colors;
        Array<uint8_t>* paletteColors = PaletteColors;
        Colors = Array<uint8_t>::Empty();
        PaletteColors = Array<uint8_t>::Empty();
        if (colors != nullptr && colors != Array<uint8_t>::Empty()) {
            delete colors;
        }
        if (paletteColors != nullptr && paletteColors != Array<uint8_t>::Empty() && paletteColors != colors) {
            delete paletteColors;
        }
        if (NativeAsset != nullptr) { NativeAsset->Dispose(); delete NativeAsset; NativeAsset = nullptr; }
    }

    /// Keeps native GPU data while supplying exact native alpha coverage to existing 2D drawing paths.
    void NintendoDsRuntimeTexture2D::LoadNative(TextureAsset* asset) {
        Colors = NintendoDsTextureCodec::Decode(asset);
        NativeAsset = new TextureAsset();
        NativeAsset->Width = asset->Width; NativeAsset->Height = asset->Height;
        NativeAsset->ColorFormat = asset->ColorFormat; NativeAsset->AlphaPrecision = asset->AlphaPrecision;
        NativeAsset->Colors = new Array<uint8_t>(asset->Colors->Length);
        Array<uint8_t>::Copy(asset->Colors, NativeAsset->Colors, asset->Colors->Length);
        if (asset->PaletteColors != nullptr && asset->PaletteColors->Length != 0) {
            NativeAsset->PaletteColors = new Array<uint8_t>(asset->PaletteColors->Length);
            Array<uint8_t>::Copy(asset->PaletteColors, NativeAsset->PaletteColors, asset->PaletteColors->Length);
        }
        ColorFormat = TextureAssetColorFormat::Rgba32; AlphaPrecision = TextureAssetAlphaPrecision::A8;
    }
}
#endif
