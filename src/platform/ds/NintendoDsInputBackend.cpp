#include "platform/ds/NintendoDsInputBackend.hpp"

extern "C" {
#include <nds/arm9/input.h>
}

#include <cstdint>

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    namespace {
        /// Stores the raw Nintendo DS hardware bit used by libnds to report an active stylus press.
        constexpr uint32_t NintendoDsTouchKeyMask = (1u << 14);
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
        int stylusDeltaX = 0;
        int stylusDeltaY = 0;
        if (stylusIsDown) {
            stylusX = stylusPosition.px;
            stylusY = stylusPosition.py;
            if (HasPreviousStylusPosition && PreviousStylusPressed) {
                stylusDeltaX = stylusX - PreviousStylusX;
                stylusDeltaY = stylusY - PreviousStylusY;
            }

            PreviousStylusX = stylusX;
            PreviousStylusY = stylusY;
            HasPreviousStylusPosition = true;
        }

        InputFrameState frame {};
        frame.Keyboard = KeyboardState();
        frame.Mouse = MouseState(
            stylusX,
            stylusY,
            0,
            stylusIsDown ? ButtonState::Pressed : ButtonState::Released,
            ButtonState::Released,
            ButtonState::Released,
            ButtonState::Released,
            ButtonState::Released);
        InputPointerState pointerState {};
        pointerState.Connected = true;
        pointerState.X = stylusX;
        pointerState.Y = stylusY;
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
