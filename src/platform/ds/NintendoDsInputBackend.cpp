#include "platform/ds/NintendoDsInputBackend.hpp"

extern "C" {
#include <nds/arm9/input.h>
}

#include <cstdint>

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Initializes the DS input backend.
    NintendoDsInputBackend::NintendoDsInputBackend() {
    }

    /// Captures one raw DS input frame and exposes it as the shared primary-gamepad state.
    /// <returns>Input frame populated from the current DS key state.</returns>
    InputFrameState NintendoDsInputBackend::CaptureFrame() {
        scanKeys();
        uint32_t heldKeys = keysHeld();

        InputFrameState frame {};
        frame.Keyboard = KeyboardState();
        frame.Mouse = MouseState(0, 0, 0, ButtonState::Released, ButtonState::Released, ButtonState::Released, ButtonState::Released, ButtonState::Released);
        frame.Pointer = InputPointerState();
        frame.Text = InputTextState();
        frame.Gamepads = new Array<InputGamepadState>(1);
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
        frame.Gamepads->Data[0] = gamepadState;
        return frame;
    }
}
#endif
