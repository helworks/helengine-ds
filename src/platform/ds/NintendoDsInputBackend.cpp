#include "platform/ds/NintendoDsInputBackend.hpp"

extern "C" {
#include <nds/arm9/input.h>
}

#include <cstdint>
#include <string>

#include "system/io/file.hpp"

#ifndef HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
#define HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS 0
#endif

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    namespace {
        /// Stores the raw Nintendo DS hardware bit used by libnds to report an active stylus press.
        constexpr uint32_t NintendoDsTouchKeyMask = (1u << 14);

        /// Stores the Nintendo DS screen height used to map bottom-screen touch into stacked dual-screen window space.
        constexpr int NintendoDsScreenHeight = 192;

#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        /// Host trace path used to capture stylus edge transitions during DS touch debugging.
        constexpr const char* TouchTracePath = "C:/tmp/helengine-ds-touch-trace.log";

        /// Indicates whether the touch trace file was already reset for the current runtime session.
        bool TouchTraceReset = false;

        /// Appends one DS touch diagnostic line to the host trace file without affecting runtime behavior on failure.
        /// <param name="line">Trace payload to append.</param>
        void AppendTouchTraceLine(const std::string& line) {
            try {
                if (!TouchTraceReset) {
                    ::File::Delete(TouchTracePath);
                    TouchTraceReset = true;
                }

                ::FileStream stream(TouchTracePath, ::FileMode::Append);
                stream.Write(reinterpret_cast<const uint8_t*>(line.data()), 0, line.size());
                uint8_t newline = static_cast<uint8_t>('\n');
                stream.Write(&newline, 0, 1);
                stream.Flush();
                stream.Close();
            } catch (...) {
            }
        }
#else
        /// Suppresses touch-edge host tracing when release-oriented DS builds disable native runtime diagnostics.
        /// <param name="line">Trace payload to ignore.</param>
        void AppendTouchTraceLine(const std::string& line) {
            (void)line;
        }
#endif
    }

    /// Initializes the DS input backend.
    NintendoDsInputBackend::NintendoDsInputBackend()
        : PrimaryCachedGamepads(new Array<InputGamepadState>(1))
        , SecondaryCachedGamepads(new Array<InputGamepadState>(1))
        , UsePrimaryCachedGamepads(true)
        , PreviousStylusPressed(false)
        , PreviousStylusX(0)
        , PreviousStylusY(0)
        , HasPreviousStylusPosition(false) {
    }

    /// Captures one raw DS input frame and exposes it as the shared primary-gamepad state.
    /// <returns>Input frame populated from the current DS key state.</returns>
    InputFrameState NintendoDsInputBackend::CaptureFrame() {
        scanKeys();
        uint32_t heldKeys = keysHeld();
        touchPosition stylusPosition {};
        touchRead(&stylusPosition);
        bool stylusIsDown = (heldKeys & NintendoDsTouchKeyMask) != 0;

        int stylusX = HasPreviousStylusPosition ? PreviousStylusX : 0;
        int stylusY = HasPreviousStylusPosition ? PreviousStylusY : 0;
        int stylusWindowX = stylusX;
        int stylusWindowY = HasPreviousStylusPosition ? PreviousStylusY + NintendoDsScreenHeight : NintendoDsScreenHeight;
        int stylusDeltaX = 0;
        int stylusDeltaY = 0;
        if (stylusIsDown) {
            stylusX = stylusPosition.px;
            stylusY = stylusPosition.py;
            stylusWindowX = stylusX;
            stylusWindowY = stylusY + NintendoDsScreenHeight;
            if (HasPreviousStylusPosition && PreviousStylusPressed) {
                stylusDeltaX = stylusX - PreviousStylusX;
                stylusDeltaY = stylusY - PreviousStylusY;
            }

            PreviousStylusX = stylusX;
            PreviousStylusY = stylusY;
            HasPreviousStylusPosition = true;
        }
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        if (stylusIsDown != PreviousStylusPressed) {
            std::string edgeKind = stylusIsDown ? "down" : "up";
            AppendTouchTraceLine(
                std::string("touch ")
                + edgeKind
                + " px="
                + std::to_string(stylusPosition.px)
                + " py="
                + std::to_string(stylusPosition.py)
                + " wx="
                + std::to_string(stylusWindowX)
                + " wy="
                + std::to_string(stylusWindowY)
                + " held="
                + std::to_string(static_cast<unsigned int>(heldKeys)));
        }
#endif

        InputFrameState frame {};
        frame.Keyboard = KeyboardState();
        frame.Mouse = MouseState(
            stylusWindowX,
            stylusWindowY,
            0,
            stylusIsDown ? ButtonState::Pressed : ButtonState::Released,
            ButtonState::Released,
            ButtonState::Released,
            ButtonState::Released,
            ButtonState::Released);
        InputPointerState pointerState {};
        pointerState.Connected = true;
        pointerState.X = stylusWindowX;
        pointerState.Y = stylusWindowY;
        pointerState.DeltaX = stylusIsDown ? stylusDeltaX : 0;
        pointerState.DeltaY = stylusIsDown ? stylusDeltaY : 0;
        pointerState.ScrollDelta = 0;
        pointerState.SetButtonDown(InputPointerButton::Primary, stylusIsDown);
        frame.Pointer = pointerState;
        frame.Text = InputTextState();
        Array<InputGamepadState>* gamepadStorage = UsePrimaryCachedGamepads ? PrimaryCachedGamepads : SecondaryCachedGamepads;
        frame.Gamepads = gamepadStorage;
        frame.GamepadCount = 1;

        InputGamepadState gamepadState {};
        gamepadState.Connected = true;
        gamepadState.SetButtonDown(InputGamepadButton::DPadUp, (heldKeys & KEY_UP) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadDown, (heldKeys & KEY_DOWN) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadLeft, (heldKeys & KEY_LEFT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadRight, (heldKeys & KEY_RIGHT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::South, (heldKeys & KEY_A) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::East, (heldKeys & KEY_B) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::Start, (heldKeys & KEY_START) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::Select, (heldKeys & KEY_SELECT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::LeftShoulder, (heldKeys & KEY_L) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::RightShoulder, (heldKeys & KEY_R) != 0);
        gamepadStorage->Data[0] = gamepadState;
        UsePrimaryCachedGamepads = !UsePrimaryCachedGamepads;
        PreviousStylusPressed = stylusIsDown;
        return frame;
    }
}
#endif
