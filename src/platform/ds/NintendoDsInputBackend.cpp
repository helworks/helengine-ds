#include "platform/ds/NintendoDsInputBackend.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Initializes the DS input backend.
    NintendoDsInputBackend::NintendoDsInputBackend() {
    }

    /// Captures one default input frame with no active controls.
    /// <returns>Default input frame state.</returns>
    InputFrameState NintendoDsInputBackend::CaptureFrame() {
        return InputFrameState();
    }
}
#endif
