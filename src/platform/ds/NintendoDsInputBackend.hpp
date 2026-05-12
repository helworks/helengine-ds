#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "IInputBackend.hpp"
#include "InputFrameState.hpp"

namespace helengine::ds {
    /// Provides the minimal Nintendo DS input backend needed to initialize generated core.
    class NintendoDsInputBackend : public IInputBackend {
    public:
        /// <summary>
        /// Captures one default input frame with no active controls.
        /// </summary>
        /// <returns>Default input frame state.</returns>
        InputFrameState CaptureFrame() override;
    };
}
#endif
