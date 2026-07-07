#include "feature_manifest.hpp"

static const HEFeatureEntry kFeatureEntries[] = {
    { HEFeature::DebugOverlay, false, HEFeatureDecisionOrigin::ForcedDisabled, "debug_overlay" },
    { HEFeature::HostFileSystem, true, HEFeatureDecisionOrigin::AutoDetected, "host_file_system" },
    { HEFeature::ReflectionLikeRuntime, false, HEFeatureDecisionOrigin::NotIncluded, "reflection_like_runtime" },
    { HEFeature::Render2d, true, HEFeatureDecisionOrigin::AutoDetected, "render2d" },
    { HEFeature::RuntimeJson, false, HEFeatureDecisionOrigin::NotIncluded, "runtime_json" },
    { HEFeature::Shaders, false, HEFeatureDecisionOrigin::NotIncluded, "shaders" },
    { HEFeature::Sprites, true, HEFeatureDecisionOrigin::AutoDetected, "sprites" },
    { HEFeature::Text2d, true, HEFeatureDecisionOrigin::AutoDetected, "text2d" },
    { HEFeature::TextProcessing, true, HEFeatureDecisionOrigin::AutoDetected, "text_processing" },
};

bool he_feature_enabled(HEFeature feature) {
    for (const HEFeatureEntry& entry : kFeatureEntries) {
        if (entry.Feature == feature) {
            return entry.Enabled;
        }
    }

    return false;
}

const HEFeatureEntry* he_get_feature_entries(std::size_t* count) {
    if (count != nullptr) {
        *count = sizeof(kFeatureEntries) / sizeof(kFeatureEntries[0]);
    }

    return kFeatureEntries;
}
