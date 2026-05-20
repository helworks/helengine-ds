namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS input backend source so generated-core menu navigation receives the same abstract gamepad buttons used by other handheld runtimes.
/// </summary>
public class NintendoDsInputBackendSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS input backend scans held hardware keys and translates them into the shared primary-gamepad contract.
    /// </summary>
    [Fact]
    public void Source_whenCapturingOneInputFrame_mapsDsButtonsToSharedPrimaryGamepadButtons() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("InputFrameState CaptureFrame() override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("Array<InputGamepadState>* PrimaryCachedGamepads;", headerSource, StringComparison.Ordinal);
        Assert.Contains("Array<InputGamepadState>* SecondaryCachedGamepads;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UsePrimaryCachedGamepads;", headerSource, StringComparison.Ordinal);
        Assert.Contains("scanKeys();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t heldKeys = keysHeld();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrimaryCachedGamepads(new Array<InputGamepadState>(1))", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SecondaryCachedGamepads(new Array<InputGamepadState>(1))", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UsePrimaryCachedGamepads(true)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Array<InputGamepadState>* gamepadStorage = UsePrimaryCachedGamepads ? PrimaryCachedGamepads : SecondaryCachedGamepads;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.Gamepads = gamepadStorage;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.GamepadCount = 1;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InputGamepadState gamepadState {};", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.Connected = true;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadUp, (heldKeys & KEY_UP) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadDown, (heldKeys & KEY_DOWN) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadLeft, (heldKeys & KEY_LEFT) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadRight, (heldKeys & KEY_RIGHT) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::South, (heldKeys & KEY_A) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::East, (heldKeys & KEY_B) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::Start, (heldKeys & KEY_START) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::Select, (heldKeys & KEY_SELECT) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::LeftShoulder, (heldKeys & KEY_L) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::RightShoulder, (heldKeys & KEY_R) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadStorage->Data[0] = gamepadState;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UsePrimaryCachedGamepads = !UsePrimaryCachedGamepads;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("frame.Gamepads = new Array<InputGamepadState>(1);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS input backend initializes the full frame contract so menu hover logic never consumes undefined pointer or keyboard state.
    /// </summary>
    [Fact]
    public void Source_whenCapturingOneInputFrame_mapsStylusStateToSharedMouseAndPointerContracts() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool PreviousStylusPressed;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int PreviousStylusX;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int PreviousStylusY;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool HasPreviousStylusPosition;", headerSource, StringComparison.Ordinal);
        Assert.Contains("touchPosition stylusPosition {};", sourceCode, StringComparison.Ordinal);
        Assert.Contains("touchRead(&stylusPosition);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool stylusIsDown = (heldKeys & KEY_TOUCH) != 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int stylusX = HasPreviousStylusPosition ? PreviousStylusX : 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int stylusY = HasPreviousStylusPosition ? PreviousStylusY : 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.Mouse = MouseState(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("stylusIsDown ? ButtonState::Pressed : ButtonState::Released", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InputPointerState pointerState {};", sourceCode, StringComparison.Ordinal);
        Assert.Contains("pointerState.Connected = true;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("pointerState.SetButtonDown(InputPointerButton::Primary, stylusIsDown);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.Pointer = pointerState;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PreviousStylusPressed = stylusIsDown;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS input backend preserves the last touch position and emits zero pointer delta after release.
    /// </summary>
    [Fact]
    public void Source_whenCapturingOneInputFrame_preservesLastTouchPositionAndZerosDeltaWhenStylusIsReleased() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("if (stylusIsDown) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (HasPreviousStylusPosition && PreviousStylusPressed) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("stylusDeltaX = stylusX - PreviousStylusX;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("stylusDeltaY = stylusY - PreviousStylusY;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("pointerState.DeltaX = stylusIsDown ? stylusDeltaX : 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("pointerState.DeltaY = stylusIsDown ? stylusDeltaY : 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.Text = InputTextState();", sourceCode, StringComparison.Ordinal);
    }
}
