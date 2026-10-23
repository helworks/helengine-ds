#include "platform/ds/NintendoDsLightingMath.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <algorithm>
#include <cmath>

#include "platform/ds/NintendoDsColorPacker.hpp"
#include "RealMath.hpp"

namespace helengine::ds {
    RealEvaluation NintendoDsLightingMath::EvaluateDirectionalDiffuse(const float3& normal, const float3& lightDirection) {
        float3 normalizedNormal = float3::Normalize(normal);
        float3 normalizedLightDirection = float3::Normalize(lightDirection);
        RealEvaluation diffuse = float3::Dot(normalizedNormal, normalizedLightDirection * -1.0f);
        if (diffuse < 0.0f) {
            diffuse = 0.0f;
        }

        return RealMath::Clamp(diffuse, 0.0f, 1.0f);
    }

    RealEvaluation NintendoDsLightingMath::ComputeLuminance(const float3& radiance) {
        return RealMath::Clamp(
            (radiance.X * 0.2126f) +
            (radiance.Y * 0.7152f) +
            (radiance.Z * 0.0722f),
            0.0f,
            1.0f);
    }

    RealEvaluation NintendoDsLightingMath::ApplyDisplayContrastCurve(RealEvaluation intensity) {
        RealEvaluation clampedIntensity = RealMath::Clamp(intensity, 0.0f, 1.0f);
        return RealEvaluation(std::pow(
            he_cpp_real_plain_value(clampedIntensity.Value),
            he_cpp_real_plain_value(he_real_eval(0.85f))));
    }

    float3 NintendoDsLightingMath::ApplyDisplayContrastCurve(const float3& color) {
        return float3(
            ApplyDisplayContrastCurve(RealEvaluation(color.X)),
            ApplyDisplayContrastCurve(RealEvaluation(color.Y)),
            ApplyDisplayContrastCurve(RealEvaluation(color.Z)));
    }

    float3 NintendoDsLightingMath::ClampColor(const float3& color) {
        return float3(
            RealMath::Clamp(color.X, 0.0f, 1.0f),
            RealMath::Clamp(color.Y, 0.0f, 1.0f),
            RealMath::Clamp(color.Z, 0.0f, 1.0f));
    }

    float3 NintendoDsLightingMath::MultiplyColor(const float3& left, const float3& right) {
        return float3(
            left.X * right.X,
            left.Y * right.Y,
            left.Z * right.Z);
    }

    uint16_t NintendoDsLightingMath::ScalePackedGreyscale(RealEvaluation intensity) {
        float clampedIntensity = static_cast<float>(RealMath::Clamp(intensity, 0.0f, 1.0f));
        return NintendoDsColorPacker::PackOpaqueColor(clampedIntensity, clampedIntensity, clampedIntensity);
    }

    float3 NintendoDsLightingMath::ComputeTriangleNormal(const float3& a, const float3& b, const float3& c) {
        float3 edgeA = b - a;
        float3 edgeB = c - a;
        float3 normal = float3::Cross(edgeA, edgeB);
        RealEvaluation lengthSquared = float3::Dot(normal, normal);
        if (lengthSquared <= 0.000001f) {
            return float3(0.0f, 0.0f, 1.0f);
        }

        return float3::Normalize(normal);
    }
}
#endif
