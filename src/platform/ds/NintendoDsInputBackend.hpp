#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "IInputBackend.hpp"
#include "InputGamepadButton.hpp"
#include "InputGamepadState.hpp"
#include "InputFrameState.hpp"

namespace helengine::ds {
    /// Translates Nintendo DS hardware buttons into the shared generated-core input contract.
    class NintendoDsInputBackend : public IInputBackend {
    public:
        /// Initializes the DS input backend.
        NintendoDsInputBackend();

        /// <summary>
        /// Captures one raw DS input frame and exposes it as the shared primary-gamepad state.
        /// </summary>
        /// <returns>Input frame populated from the current DS key state.</returns>
        InputFrameState CaptureFrame() override;
    };
}
#endif
