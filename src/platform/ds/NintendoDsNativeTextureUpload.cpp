#include "platform/ds/NintendoDsNativeTextureUpload.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
extern "C" {
#include <nds/arm9/videoGL.h>
}
#include <memory>
#include <stdexcept>
#include <vector>
#include "TextureAsset.hpp"
#include "NintendoDsTextureCodec.hpp"
#include "NintendoDsTextureLayout.hpp"
#include "NintendoDsTextureFormat.hpp"
#include "runtime/array.hpp"

namespace helengine::ds {
    namespace {
        /// Converts a validated storage extent to the libnds texture-size exponent.
        int Size(int value) { int result = 0; while (value > 8) { value /= 2; ++result; } return result; }
    }
    /// Preserves native 3D formats; 2D tile indices are untiled into their corresponding GPU indexed encoding.
    int NintendoDsNativeTextureUpload::Upload(TextureAsset* asset) {
        std::unique_ptr<NintendoDsTextureLayout> layout(NintendoDsTextureCodec::ReadLayout(asset));
        const int code = layout->Format->Code;
        const int hardwareCode = code == 8 ? 3 : code == 9 ? 4 : code;
        const int bits = layout->Format->BitsPerPixel;
        const uint8_t* texels = asset->Colors->Data + NintendoDsTextureCodec::HeaderLength;
        std::vector<uint8_t> linear;
        if (layout->Format->get_IsTiled()) {
            linear.assign(static_cast<std::size_t>(layout->TexelBytes), 0);
            for (int y = 0; y < layout->StorageHeight; ++y) for (int x = 0; x < layout->StorageWidth; ++x) {
                const int source = NintendoDsTextureLayout::PixelIndex(x, y, layout->StorageWidth, true);
                const int target = y * layout->StorageWidth + x;
                const int value = bits == 8 ? texels[source] : (texels[source / 2] >> (source % 2 * 4)) & 15;
                linear[static_cast<std::size_t>(target * bits / 8)] |= static_cast<uint8_t>(value << (bits == 8 ? 0 : target % 2 * 4));
            }
            texels = linear.data();
        }
        int textureId = -1;
        if (glGenTextures(1, &textureId) == 0 || textureId <= 0) throw std::runtime_error("Nintendo DS native texture name allocation failed.");
        glBindTexture(0, textureId);
        try {
            const int flags = TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T
                | (NintendoDsTextureCodec::Read(asset->Colors, 40) != 0 ? GL_TEXTURE_COLOR0_TRANSPARENT : 0);
            if (glTexImage2D(0, 0, static_cast<GL_TEXTURE_TYPE_ENUM>(hardwareCode), Size(layout->StorageWidth), Size(layout->StorageHeight), 0, flags, texels) == 0) {
                throw std::runtime_error("Nintendo DS native texels do not fit the configured texture VRAM.");
            }
            if (asset->PaletteColors != nullptr && asset->PaletteColors->Length != 0) {
                const int entries = asset->PaletteColors->Length / 2;
                glColorTableEXT(0, 0, static_cast<uint16_t>(entries), 0, 0, reinterpret_cast<const uint16_t*>(asset->PaletteColors->Data));
                int uploaded = -1; glGetColorTableParameterEXT(0, GL_COLOR_TABLE_WIDTH_EXT, &uploaded);
                if (uploaded < entries) throw std::runtime_error("Nintendo DS native palette does not fit the configured palette VRAM.");
            }
        } catch (...) { glDeleteTextures(1, &textureId); throw; }
        return textureId;
    }
}
#endif
