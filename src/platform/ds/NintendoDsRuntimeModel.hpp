#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

#include "RuntimeModel.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"

namespace helengine::ds {
    /// Carries the authored vertex and index data needed for DS geometry submission.
    class NintendoDsRuntimeModel : public RuntimeModel {
    public:
        /// Creates one empty DS runtime model.
        NintendoDsRuntimeModel();

        /// Authored vertex positions preserved from the cooked model asset.
        Array<float3>* Positions;

        /// Authored vertex texture coordinates preserved from the cooked model asset.
        Array<float2>* TexCoords;

        /// Authored 16-bit indices when present.
        Array<uint16_t>* Indices16;

        /// Authored 32-bit indices when present.
        Array<uint32_t>* Indices32;

        /// Packed Nintendo DS FIFO command stream used for fixed-function lit geometry submission.
        uint32_t* HardwareLitDisplayList;

        /// Number of command-stream words stored after the display-list length word.
        uint32_t HardwareLitDisplayListWordCount;

        /// True when the packed command stream uses quad primitives instead of triangle primitives.
        bool UsesHardwareLitQuadDisplayList;

        /// Packed Nintendo DS FIFO command stream used for fixed-function textured geometry submission.
        uint32_t* HardwareTexturedDisplayList;

        /// Number of textured command-stream words stored after the display-list length word.
        uint32_t HardwareTexturedDisplayListWordCount;

        /// Texture width used when the cached textured display list was generated.
        int32_t HardwareTexturedDisplayListTextureWidth;

        /// Texture height used when the cached textured display list was generated.
        int32_t HardwareTexturedDisplayListTextureHeight;

        /// True when the cached textured display list uses quad primitives instead of triangle primitives.
        bool UsesHardwareTexturedQuadDisplayList;

        /// True when the authored model kept 32-bit indices.
        bool Uses32BitIndices;
    };
}
#endif
