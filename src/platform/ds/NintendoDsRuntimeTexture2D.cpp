#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Creates one DS runtime texture with no prepared hardware payload yet.
    NintendoDsRuntimeTexture2D::NintendoDsRuntimeTexture2D()
        : ColorFormat(TextureAssetColorFormat::Rgba32)
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
}
#endif
