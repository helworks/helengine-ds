#include "platform/ds/NintendoDsRuntimeMaterial.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Creates one DS runtime material with visible defaults for the first renderer slice.
    NintendoDsRuntimeMaterial::NintendoDsRuntimeMaterial()
        : PackedDiffuseColor(0xFFFF)
        , SupportsGeometrySubmission(false) {
    }
}
#endif
