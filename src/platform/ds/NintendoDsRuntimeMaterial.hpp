#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

#include "RuntimeMaterial.hpp"

namespace helengine::ds {
    /// Carries the minimal DS-owned material state needed for the first visible opaque geometry path.
    class NintendoDsRuntimeMaterial : public RuntimeMaterial {
    public:
        /// Creates one DS runtime material with visible defaults for the first renderer slice.
        NintendoDsRuntimeMaterial();

        /// Packed BGR5A1 color reserved for the DS polygon path.
        uint16_t PackedDiffuseColor;

        /// True when the first DS renderer slice may submit this material.
        bool SupportsGeometrySubmission;
    };
}
#endif
