#include "platform/ds/NintendoDsInputBackend.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Initializes the DS input backend with background input disabled.
    NintendoDsInputBackend::NintendoDsInputBackend()
        : ReceiveInputInBackground(false) {
    }

    /// Gets whether the backend should keep reporting input while the host is not foregrounded.
    bool NintendoDsInputBackend::get_ReceiveInputInBackground() {
        return ReceiveInputInBackground;
    }

    /// Sets whether the backend should keep reporting input while the host is not foregrounded.
    void NintendoDsInputBackend::set_ReceiveInputInBackground(bool value) {
        ReceiveInputInBackground = value;
    }

    /// Captures one default input frame with no active controls.
    /// <returns>Default input frame state.</returns>
    InputFrameState NintendoDsInputBackend::CaptureFrame() {
        return InputFrameState();
    }
}
#endif
