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

    private:
        /// Stores one reusable single-gamepad array for frames that need to preserve the previous button edge state.
        Array<InputGamepadState>* PrimaryCachedGamepads;

        /// Stores the alternating reusable single-gamepad array so current and previous frames never alias the same storage.
        Array<InputGamepadState>* SecondaryCachedGamepads;

        /// Selects which cached gamepad array receives the next captured DS input frame.
        bool UsePrimaryCachedGamepads;
    };
}
#endif
