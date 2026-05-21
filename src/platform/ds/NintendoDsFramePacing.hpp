#pragma once

#include <cstdint>

namespace helengine::ds {
    /// Returns the visible-screen VBlank count maintained by the Nintendo DS boot host IRQ handler.
    uint32_t GetNintendoDsVBlankCount();
}
