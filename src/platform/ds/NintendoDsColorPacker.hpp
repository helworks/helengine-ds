#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

namespace helengine::ds {
    /// Converts normalized Helengine colors into Nintendo DS packed BGR5A1 values.
    class NintendoDsColorPacker {
    public:
        /// Packs one normalized floating-point channel into the 5-bit range used by the Nintendo DS.
        static uint16_t PackChannel(float value);

        /// Packs one normalized RGB color into DS BGR5A1 format with the visible bit enabled.
        static uint16_t PackOpaqueColor(float red, float green, float blue);

        /// Packs one default opaque white color for the first DS geometry path.
        static uint16_t PackOpaqueWhite();
    };
}
#endif
