#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "RuntimeTexture.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "TextureAssetColorFormat.hpp"
#include "runtime/array.hpp"

namespace helengine::ds {
    /// Carries one DS-owned software texture copy used by the top-screen bitmap 2D renderer.
    class NintendoDsRuntimeTexture2D : public RuntimeTexture {
    public:
        /// Creates one DS software texture with no copied pixel payload yet.
        NintendoDsRuntimeTexture2D();

        /// Cooked texture color format describing how the stored payload bytes should be decoded.
        TextureAssetColorFormat ColorFormat;

        /// Cooked texture alpha precision authored by the editor-side texture processor.
        TextureAssetAlphaPrecision AlphaPrecision;

        /// Cooked pixel payload copied from the authored runtime texture asset.
        Array<uint8_t>* Colors;

        /// Optional cooked palette payload used by indexed texture formats.
        Array<uint8_t>* PaletteColors;
    };
}
#endif
