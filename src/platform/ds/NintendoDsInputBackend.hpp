#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "IInputBackend.hpp"
#include "InputFrameState.hpp"

namespace helengine::ds {
    /// Provides the minimal Nintendo DS input backend needed to initialize generated core.
    class NintendoDsInputBackend : public IInputBackend {
    public:
        /// Initializes the DS input backend with background input disabled.
        NintendoDsInputBackend();

        /// Gets whether the backend should keep reporting input while the host is not foregrounded.
        bool get_ReceiveInputInBackground() override;

        /// Sets whether the backend should keep reporting input while the host is not foregrounded.
        void set_ReceiveInputInBackground(bool value) override;

        /// <summary>
        /// Captures one default input frame with no active controls.
        /// </summary>
        /// <returns>Default input frame state.</returns>
        InputFrameState CaptureFrame() override;

    private:
        /// Stores whether background input is enabled for the backend.
        bool ReceiveInputInBackground;
    };
}
#endif
