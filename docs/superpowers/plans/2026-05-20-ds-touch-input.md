# Nintendo DS Touch Input Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add Nintendo DS stylus input that behaves like shared mouse input and shared pointer input across the engine.

**Architecture:** Keep the change at the DS platform boundary by teaching `NintendoDsInputBackend` to translate one stylus sample into both `MouseState` and `InputPointerState`. Preserve the existing shared engine contracts so `MenuComponent` and `PointerInteractionSystem` work unchanged, and lock the behavior with focused DS source-audit tests.

**Tech Stack:** C++, libnds input APIs, generated-core input types, xUnit source-audit tests, `dotnet test`, `rtk`, melonDS.

---

### Task 1: Add Failing DS Touch Contract Audits

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing source-audit expectations for stylus-to-mouse and stylus-to-pointer mapping**

Replace the current neutral-frame-only assertions with explicit touch-contract expectations and add one new test method:

```csharp
    /// <summary>
    /// Verifies the Nintendo DS input backend captures stylus state and maps it into the shared mouse and pointer contracts.
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
```

Update the existing initialization test to assert the touch path instead of the old permanently-neutral pointer path:

```csharp
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
```

- [ ] **Step 2: Run the focused DS backend audit slice and verify it fails**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" -v minimal
```

Expected: FAIL with one or more `Assert.Contains()` failures for missing stylus fields, `touchRead(&stylusPosition);`, or `pointerState.SetButtonDown(InputPointerButton::Primary, stylusIsDown);`.

- [ ] **Step 3: Commit the red test state**

```bash
git add C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs
git commit -m "Add failing DS touch input audits"
```

### Task 2: Implement Stylus Translation in NintendoDsInputBackend

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.cpp`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs`

- [ ] **Step 1: Add cached stylus fields to the DS backend header**

Update the private section of `NintendoDsInputBackend`:

```cpp
    private:
        /// Stores one reusable single-gamepad array for frames that need to preserve the previous button edge state.
        Array<InputGamepadState>* PrimaryCachedGamepads;

        /// Stores the alternating reusable single-gamepad array so current and previous frames never alias the same storage.
        Array<InputGamepadState>* SecondaryCachedGamepads;

        /// Selects which cached gamepad array receives the next captured DS input frame.
        bool UsePrimaryCachedGamepads;

        /// Stores whether the previous captured stylus frame was pressed.
        bool PreviousStylusPressed;

        /// Stores the previous stylus X coordinate used to calculate deltas.
        int PreviousStylusX;

        /// Stores the previous stylus Y coordinate used to calculate deltas.
        int PreviousStylusY;

        /// Stores whether the backend has observed at least one valid stylus coordinate yet.
        bool HasPreviousStylusPosition;
```

- [ ] **Step 2: Initialize the cached stylus fields in the constructor**

Update the constructor initializer list in `NintendoDsInputBackend.cpp`:

```cpp
    NintendoDsInputBackend::NintendoDsInputBackend()
        : PrimaryCachedGamepads(new Array<InputGamepadState>(1))
        , SecondaryCachedGamepads(new Array<InputGamepadState>(1))
        , UsePrimaryCachedGamepads(true)
        , PreviousStylusPressed(false)
        , PreviousStylusX(0)
        , PreviousStylusY(0)
        , HasPreviousStylusPosition(false) {
    }
```

- [ ] **Step 3: Replace the neutral mouse/pointer frame setup with stylus-backed shared input mapping**

Replace the current neutral mouse/pointer setup at the top of `CaptureFrame()` with the following structure:

```cpp
    InputFrameState NintendoDsInputBackend::CaptureFrame() {
        scanKeys();
        uint32_t heldKeys = keysHeld();
        touchPosition stylusPosition {};
        touchRead(&stylusPosition);
        bool stylusIsDown = (heldKeys & KEY_TOUCH) != 0;

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
```

Leave the existing gamepad mapping logic intact below that block, and add the state write before returning:

```cpp
        gamepadStorage->Data[0] = gamepadState;
        UsePrimaryCachedGamepads = !UsePrimaryCachedGamepads;
        PreviousStylusPressed = stylusIsDown;
        return frame;
```

- [ ] **Step 4: Run the focused DS backend audit slice and verify it passes**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" -v minimal
```

Expected: PASS with `Failed: 0, Passed: 3` or the current total for that class after the new tests are added.

- [ ] **Step 5: Commit the DS touch backend implementation**

```bash
git add C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.hpp C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.cpp C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs
git commit -m "Add DS stylus mouse and pointer input"
```

### Task 3: Run a DS Touch Smoke Test

**Files:**
- Modify: none
- Verify: `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

- [ ] **Step 1: Build the DS project with the updated backend**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-editor-font\bin\' -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: `Build completed for platform 'ds': C:\tmp\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 2: Launch melonDS with the fresh ROM**

Run:

```powershell
$processes = Get-Process melonDS -ErrorAction SilentlyContinue; if ($processes) { $processes | Stop-Process -Force }; Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'
```

Expected: melonDS opens the rebuilt ROM.

- [ ] **Step 3: Manually verify menu tapping on the bottom screen**

Manual checklist:

```text
1. Tap a menu row once and confirm the selected row changes immediately.
2. Tap the currently selected row again and confirm the action triggers.
3. Drag lightly across rows and confirm hover/selection follows the stylus without requiring D-pad input.
4. Release off-row and confirm no unintended activation occurs.
```

- [ ] **Step 4: If the smoke test passes without follow-up edits, create the final feature commit**

```bash
git commit --allow-empty -m "Verify DS touch input smoke test"
```
