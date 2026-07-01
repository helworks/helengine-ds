#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "RuntimeTexture.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "TextureAssetColorFormat.hpp"
#include "runtime/array.hpp"

namespace helengine::ds {
    /// Carries one DS-owned runtime texture together with any prepared hardware sprite payloads.
    class NintendoDsRuntimeTexture2D : public RuntimeTexture {
    public:
        /// Creates one DS runtime texture with no prepared hardware payload yet.
        NintendoDsRuntimeTexture2D();

        /// Cooked texture color format describing how the stored payload bytes should be decoded.
        TextureAssetColorFormat ColorFormat;

        /// Cooked texture alpha precision authored by the editor-side texture processor.
        TextureAssetAlphaPrecision AlphaPrecision;

        /// Cooked pixel payload copied from the authored runtime texture asset.
        Array<uint8_t>* Colors;

        /// Optional cooked palette payload used by indexed texture formats.
        Array<uint8_t>* PaletteColors;

        /// Libnds texture name allocated after this texture is first bound by the 3D renderer.
        int HardwareTextureId;

        /// True once the 3D renderer has uploaded this texture payload into DS texture VRAM.
        bool HardwareTextureUploaded;

        /// DS OBJ graphics payloads allocated for top-screen tiled sprite submission.
        std::vector<void*> MainHardwareSpriteGraphics;

        /// True once the plain top-screen OBJ sprite payload has been prepared for DS submission.
        bool MainHardwareSpritePrepared;

        /// True when the prepared top-screen OBJ payload uses the DS 256-color sprite path.
        bool MainHardwareSpriteUses256Color;

        /// DS OBJ palette bank reserved for top-screen paletted sprite submission, or <c>-1</c> when no bank has been assigned.
        int32_t MainHardwareSpritePaletteBank;

        /// DS OBJ graphics payloads allocated for bottom-screen tiled sprite submission.
        std::vector<void*> SubHardwareSpriteGraphics;

        /// True once the plain bottom-screen OBJ sprite payload has been prepared for DS submission.
        bool SubHardwareSpritePrepared;

        /// True when the prepared bottom-screen OBJ payload uses the DS 256-color sprite path.
        bool SubHardwareSpriteUses256Color;

        /// DS OBJ palette bank reserved for bottom-screen paletted sprite submission, or <c>-1</c> when no bank has been assigned.
        int32_t SubHardwareSpritePaletteBank;

        /// Number of DS OBJ tiles prepared for one plain top-screen sprite submission.
        int32_t MainHardwareSpriteTileCount;

        /// Number of DS OBJ tiles prepared for one plain bottom-screen sprite submission.
        int32_t SubHardwareSpriteTileCount;
    };
}
#endif
