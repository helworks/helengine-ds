#include "platform/ds/NintendoDsColorPacker.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <algorithm>

namespace helengine::ds {
    /// Packs one normalized floating-point channel into the 5-bit range used by the Nintendo DS.
    uint16_t NintendoDsColorPacker::PackChannel(float value) {
        float clampedValue = std::clamp(value, 0.0f, 1.0f);
        return static_cast<uint16_t>(clampedValue * 31.0f + 0.5f);
    }

    /// Packs one normalized RGB color into DS BGR5A1 format with the visible bit enabled.
    uint16_t NintendoDsColorPacker::PackOpaqueColor(float red, float green, float blue) {
        uint16_t packedRed = PackChannel(red);
        uint16_t packedGreen = static_cast<uint16_t>(PackChannel(green) << 5);
        uint16_t packedBlue = static_cast<uint16_t>(PackChannel(blue) << 10);
        return static_cast<uint16_t>(packedRed | packedGreen | packedBlue);
    }

    /// Packs one default opaque white color for the first DS geometry path.
    uint16_t NintendoDsColorPacker::PackOpaqueWhite() {
        return PackOpaqueColor(1.0f, 1.0f, 1.0f);
    }
}
#endif
