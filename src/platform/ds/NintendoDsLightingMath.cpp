#include "platform/ds/NintendoDsLightingMath.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <algorithm>
#include <cmath>

#include "platform/ds/NintendoDsColorPacker.hpp"

namespace helengine::ds {
    float NintendoDsLightingMath::EvaluateDirectionalDiffuse(const float3& normal, const float3& lightDirection) {
        float3 normalizedNormal = float3::Normalize(normal);
        float3 normalizedLightDirection = float3::Normalize(lightDirection);
        float diffuse = float3::Dot(normalizedNormal, normalizedLightDirection * -1.0f);
        if (diffuse < 0.0f) {
            diffuse = 0.0f;
        }

        return std::clamp(diffuse, 0.0f, 1.0f);
    }

    float NintendoDsLightingMath::ComputeLuminance(const float3& radiance) {
        return std::clamp(
            (radiance.X * 0.2126f) +
            (radiance.Y * 0.7152f) +
            (radiance.Z * 0.0722f),
            0.0f,
            1.0f);
    }

    float NintendoDsLightingMath::ApplyDisplayContrastCurve(float intensity) {
        float clampedIntensity = std::clamp(intensity, 0.0f, 1.0f);
        return std::pow(clampedIntensity, 0.85f);
    }

    uint16_t NintendoDsLightingMath::ScalePackedGreyscale(float intensity) {
        float clampedIntensity = std::clamp(intensity, 0.0f, 1.0f);
        return NintendoDsColorPacker::PackOpaqueColor(clampedIntensity, clampedIntensity, clampedIntensity);
    }

    float3 NintendoDsLightingMath::ComputeTriangleNormal(const float3& a, const float3& b, const float3& c) {
        float3 edgeA = b - a;
        float3 edgeB = c - a;
        float3 normal = float3::Cross(edgeA, edgeB);
        float lengthSquared = float3::Dot(normal, normal);
        if (lengthSquared <= 0.000001f) {
            return float3(0.0f, 0.0f, 1.0f);
        }

        return float3::Normalize(normal);
    }
}
#endif
