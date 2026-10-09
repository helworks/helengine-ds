#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
class TextureAsset;

namespace helengine::ds {
    /// Uploads validated native texture encodings without expanding their GPU texels to direct color.
    class NintendoDsNativeTextureUpload {
    public:
        /// Returns an owned libnds texture name, releasing allocations if texel or palette upload fails.
        static int Upload(TextureAsset* asset);
    };
}
#endif
