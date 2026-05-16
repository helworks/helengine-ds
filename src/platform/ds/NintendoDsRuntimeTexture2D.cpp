#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Creates one DS software texture with no copied pixel payload yet.
    NintendoDsRuntimeTexture2D::NintendoDsRuntimeTexture2D()
        : ColorFormat(TextureAssetColorFormat::Rgba32)
        , AlphaPrecision(TextureAssetAlphaPrecision::A8)
        , Colors(Array<uint8_t>::Empty())
        , PaletteColors(Array<uint8_t>::Empty()) {
    }
}
#endif
