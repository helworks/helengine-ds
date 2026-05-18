# DS Menu Navigation Input Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the Nintendo DS main menu respond to `D-pad Up/Down` for selection movement and `A/B` for confirm/back by bringing the DS input backend into parity with the existing shared menu gamepad contract.

**Architecture:** Keep `MenuComponent` as the single owner of menu navigation behavior. Prove the shared menu runtime contract with focused gamepad tests in `helengine`, then implement the missing DS raw-frame capture in `NintendoDsInputBackend` and lock it down with DS source-audit coverage in `helengine-ds`.

**Tech Stack:** C#, xUnit, shared `InputSystem` / `InputGamepadState`, Nintendo DS native C++ backend, libnds keypad API, DS builder source-audit tests.

---

### Task 1: Prove The Shared Menu Gamepad Contract

**Files:**
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/testing/TestInputBackend.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs`

- [ ] **Step 1: Write the failing runtime menu gamepad tests**

Add two focused tests beside the existing keyboard menu tests:

```csharp
[Fact]
public void Load_WhenMenuReceivesGamepadConfirm_OpensTheSelectedPanel() {
    string projectRootPath = Path.Combine(TempRootPath, "menu-gamepad-confirm-project");
    string buildRootPath = PackageDemoMenuScene(projectRootPath, "menu-gamepad-confirm-build");
    MenuComponent menuHostComponent = LoadPackagedMenu(buildRootPath);
    TestInputBackend input = Assert.IsType<TestInputBackend>(Core.Instance.InputSystem.Backend);

    input.SetGamepadStates(Array.Empty<InputGamepadState>());
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    InputGamepadState pressedState = CreateConnectedGamepadState(InputGamepadButton.South);
    input.SetGamepadStates(new[] { pressedState });
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    Assert.Equal("options", menuHostComponent.ActivePanelId);
}

[Fact]
public void Load_WhenMenuReceivesGamepadBack_ReturnsToThePreviousPanel() {
    string projectRootPath = Path.Combine(TempRootPath, "menu-gamepad-back-project");
    string buildRootPath = PackageDemoMenuScene(projectRootPath, "menu-gamepad-back-build");
    MenuComponent menuHostComponent = LoadPackagedMenu(buildRootPath);
    TestInputBackend input = Assert.IsType<TestInputBackend>(Core.Instance.InputSystem.Backend);

    input.SetKeyboardState(new KeyboardState(Keys.Enter));
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    input.SetKeyboardState(new KeyboardState());
    input.SetGamepadStates(Array.Empty<InputGamepadState>());
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    InputGamepadState pressedState = CreateConnectedGamepadState(InputGamepadButton.East);
    input.SetGamepadStates(new[] { pressedState });
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    Assert.Equal("main", menuHostComponent.ActivePanelId);
}
```

Add one local helper near the other test helpers:

```csharp
static InputGamepadState CreateConnectedGamepadState(params InputGamepadButton[] pressedButtons) {
    InputGamepadState state = new InputGamepadState {
        Connected = true
    };

    for (int index = 0; index < pressedButtons.Length; index++) {
        state.SetButtonDown(pressedButtons[index], true);
    }

    return state;
}
```

- [ ] **Step 2: Run the focused menu runtime tests and verify the new tests pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadConfirm_OpensTheSelectedPanel|FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadBack_ReturnsToThePreviousPanel" --no-restore -v minimal
```

Expected:
- PASS
- This confirms the shared `MenuComponent` gamepad contract is already correct and does not need new production logic.

- [ ] **Step 3: If the tests fail because the helper backend cannot represent a disconnected frame cleanly, make the minimal test-backend adjustment**

Only if required, make `SetGamepadStates` explicitly treat empty input as zero connected pads:

```csharp
public void SetGamepadStates(InputGamepadState[] states) {
    if (states == null) {
        throw new ArgumentNullException(nameof(states));
    }

    Gamepads = states;
    GamepadCount = states.Length;
}
```

Do not add any menu-specific logic here. This file is only a deterministic backend stub.

- [ ] **Step 4: Re-run the same focused tests to keep the shared contract green**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadConfirm_OpensTheSelectedPanel|FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadBack_ReturnsToThePreviousPanel" --no-restore -v minimal
```

Expected:
- PASS

- [ ] **Step 5: Commit the shared contract tests**

```powershell
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs engine/helengine.editor.tests/testing/TestInputBackend.cs
git -C C:\dev\helworks\helengine commit -m "test: lock shared menu gamepad navigation contract"
```

### Task 2: Add A DS Failing Audit For Native Gamepad Mapping

**Files:**
- Create: `C:/dev/helworks/helengine-ds/builder.tests/NintendoDsInputBackendSourceAuditTests.cs`
- Test: `C:/dev/helworks/helengine-ds/builder.tests/NintendoDsInputBackendSourceAuditTests.cs`

- [ ] **Step 1: Write the failing DS source-audit test**

Create a new audit test file:

```csharp
namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS input backend source so DS hardware buttons are normalized into the shared gamepad contract used by menu navigation.
/// </summary>
public class NintendoDsInputBackendSourceAuditTests {
    [Fact]
    public void Source_whenCapturingFrame_mapsNintendoDsButtonsIntoPrimaryGamepadState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("scanKeys();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t heldKeys = keysHeld();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InputGamepadState gamepadState;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.Connected = true;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadUp, (heldKeys & KEY_UP) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadDown, (heldKeys & KEY_DOWN) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::South, (heldKeys & KEY_A) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::East, (heldKeys & KEY_B) != 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.Gamepads = new Array<InputGamepadState>(1);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("frame.GamepadCount = 1;", sourceCode, StringComparison.Ordinal);
    }
}
```

- [ ] **Step 2: Run the new DS audit and verify it fails for the stubbed backend**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" --no-restore -v minimal
```

Expected:
- FAIL
- Failure should show that `NintendoDsInputBackend.cpp` still returns `InputFrameState()` and does not contain button mapping logic.

- [ ] **Step 3: Commit only if the team wants the explicit red-state checkpoint**

If a red-state commit is desired:

```powershell
git -C C:\dev\helworks\helengine-ds add builder.tests/NintendoDsInputBackendSourceAuditTests.cs
git -C C:\dev\helworks\helengine-ds commit -m "test: add DS input backend mapping audit"
```

If not, leave this uncommitted and continue directly to Task 3.

### Task 3: Implement Nintendo DS Input Mapping

**Files:**
- Modify: `C:/dev/helworks/helengine-ds/src/platform/ds/NintendoDsInputBackend.hpp`
- Modify: `C:/dev/helworks/helengine-ds/src/platform/ds/NintendoDsInputBackend.cpp`
- Test: `C:/dev/helworks/helengine-ds/builder.tests/NintendoDsInputBackendSourceAuditTests.cs`

- [ ] **Step 1: Add the required generated-core and libnds includes to the DS backend source**

Update the native source includes so the backend can populate shared gamepad state from Nintendo DS keys:

```cpp
#include "platform/ds/NintendoDsInputBackend.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
extern "C" {
#include <nds/arm9/input.h>
}

#include "InputGamepadButton.hpp"
#include "InputGamepadState.hpp"
```

Keep the implementation inside the existing `#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE` guard.

- [ ] **Step 2: Replace the stubbed frame capture with a single connected primary gamepad mapping**

Implement the minimal parity mapping in `NintendoDsInputBackend.cpp`:

```cpp
InputFrameState NintendoDsInputBackend::CaptureFrame() {
    scanKeys();
    uint32_t heldKeys = keysHeld();

    InputFrameState frame;
    frame.Gamepads = new Array<InputGamepadState>(1);
    frame.GamepadCount = 1;

    InputGamepadState gamepadState;
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
```

Do not add menu-specific behavior, button-edge caching, or scene knowledge here. `InputSystem` already computes pressed edges from consecutive frames.

- [ ] **Step 3: If the header needs additional documentation only, keep it minimal**

If the header comment still describes the backend as returning an empty frame, update it to match the real behavior:

```cpp
/// Captures one Nintendo DS input frame and maps hardware buttons into the shared primary-gamepad contract.
/// <returns>Raw frame state containing one connected gamepad snapshot.</returns>
InputFrameState CaptureFrame() override;
```

- [ ] **Step 4: Run the DS source-audit test and verify it passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" --no-restore -v minimal
```

Expected:
- PASS

- [ ] **Step 5: Commit the DS backend mapping**

```powershell
git -C C:\dev\helworks\helengine-ds add builder.tests/NintendoDsInputBackendSourceAuditTests.cs src/platform/ds/NintendoDsInputBackend.hpp src/platform/ds/NintendoDsInputBackend.cpp
git -C C:\dev\helworks\helengine-ds commit -m "fix: map DS buttons into shared menu gamepad input"
```

### Task 4: Verify End-To-End DS Menu Navigation

**Files:**
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs`
- Modify: `C:/dev/helworks/helengine-ds/builder.tests/NintendoDsInputBackendSourceAuditTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs`
- Test: `C:/dev/helworks/helengine-ds/builder.tests/NintendoDsInputBackendSourceAuditTests.cs`

- [ ] **Step 1: Add one shared runtime test for D-pad movement if it is missing**

Add a focused movement test near the existing menu scrolling tests:

```csharp
[Fact]
public void Load_WhenMenuReceivesGamepadDown_SelectsTheNextItem() {
    string projectRootPath = Path.Combine(TempRootPath, "menu-gamepad-down-project");
    string buildRootPath = PackageDemoMenuScene(projectRootPath, "menu-gamepad-down-build");
    MenuComponent menuHostComponent = LoadPackagedMenu(buildRootPath);
    TestInputBackend input = Assert.IsType<TestInputBackend>(Core.Instance.InputSystem.Backend);

    input.SetGamepadStates(Array.Empty<InputGamepadState>());
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    input.SetGamepadStates(new[] { CreateConnectedGamepadState(InputGamepadButton.DPadDown) });
    input.EarlyUpdate();
    menuHostComponent.Update();
    input.Update();

    Assert.Equal("select-scene", menuHostComponent.SelectedItemId);
}
```

- [ ] **Step 2: Run the focused shared and DS tests together**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadConfirm_OpensTheSelectedPanel|FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadBack_ReturnsToThePreviousPanel|FullyQualifiedName~RuntimeSceneLoadServiceTests.Load_WhenMenuReceivesGamepadDown_SelectsTheNextItem" --no-restore -v minimal
$env:HELENGINE_ROOT='C:\dev\helworks\helengine'; rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests|FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests|FullyQualifiedName~NintendoDsBootHostSourceAuditTests" --no-restore -v minimal
```

Expected:
- All focused `helengine` menu runtime tests pass
- All focused `helengine-ds` builder/source-audit tests pass

- [ ] **Step 3: Rebuild the DS ROM with the current city project**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-menu-input\bin\' -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helworks\helengine-ds-city-cube-project\output\ds
```

Expected:
- `Build completed for platform 'ds': C:\tmp\helworks\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 4: Launch melonDS for manual parity verification**

Run:

```powershell
Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helworks\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'
```

Expected manual result:
- `D-pad Up/Down` moves the selected bottom-screen menu row
- `A` activates the selected row
- `B` performs back where applicable
- top-screen logo/title remains unchanged

- [ ] **Step 5: Commit the final verification-side test additions if Task 4 added any**

```powershell
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "test: cover gamepad-driven menu navigation"
```

---

## Self-Review

- **Spec coverage:** The plan covers DS-only backend parity, shared menu contract verification, `D-pad Up/Down`, `A`, and `B`, plus DS rebuild and emulator verification. No spec requirement is left without a task.
- **Placeholder scan:** No `TODO`, `TBD`, or vague “add tests” steps remain; each task names exact files, commands, and code snippets.
- **Type consistency:** The plan uses the existing shared types and names consistently: `InputFrameState`, `InputGamepadState`, `InputGamepadButton`, `MenuComponent`, `NintendoDsInputBackend`, and `WasGamepadButtonPressed`.
