#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

#include "ShaderRuntimeMaterial.hpp"
#include "float3.hpp"

namespace helengine::ds {
    /// Carries the minimal DS-owned material state needed for the first visible opaque geometry path.
    class NintendoDsRuntimeMaterial : public ShaderRuntimeMaterial {
    public:
        /// Creates one DS runtime material with visible defaults for the first renderer slice.
        NintendoDsRuntimeMaterial();

        /// Packed BGR5A1 color reserved for the DS polygon path.
        uint16_t PackedDiffuseColor;

        /// Normalized authored base RGB color resolved from the standard-material constant buffer.
        float3 BaseColor;

        /// True when the first DS renderer slice may submit this material.
        bool SupportsGeometrySubmission;

        /// True when the DS fixed-function pipeline should apply scene lighting to this material.
        bool LightingEnabled;
    };
}
#endif
