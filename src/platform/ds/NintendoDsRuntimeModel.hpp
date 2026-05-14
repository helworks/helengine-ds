#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

#include "RuntimeModel.hpp"
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

        /// Authored 16-bit indices when present.
        Array<uint16_t>* Indices16;

        /// Authored 32-bit indices when present.
        Array<uint32_t>* Indices32;

        /// True when the authored model kept 32-bit indices.
        bool Uses32BitIndices;
    };
}
#endif
