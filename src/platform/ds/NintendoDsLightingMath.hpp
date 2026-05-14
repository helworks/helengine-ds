#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

#include "float3.hpp"

namespace helengine::ds {
    /// Provides the minimal face-lighting math needed by the DS cube-test renderer.
    class NintendoDsLightingMath {
    public:
        /// Evaluates one clamped directional diffuse term.
        static float EvaluateDirectionalDiffuse(const float3& normal, const float3& lightDirection);

        /// Converts one linear RGB radiance triplet into a scalar luminance for grayscale lighting.
        static float ComputeLuminance(const float3& radiance);

        /// Shapes one lit intensity for clearer mid-tone separation on the DS display.
        static float ApplyDisplayContrastCurve(float intensity);

        /// Packs one scalar greyscale intensity into DS BGR5 polygon color.
        static uint16_t ScalePackedGreyscale(float intensity);

        /// Computes one normalized face normal from three world-space triangle vertices.
        static float3 ComputeTriangleNormal(const float3& a, const float3& b, const float3& c);
    };
}
#endif
