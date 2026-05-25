# DS Return Overlay Ownership Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make DS companion-scene return behavior owned by one DS-specific overlay controller, fix bottom-screen touch hit testing, and preserve strict scene-load behavior.

**Architecture:** The DS platform boundary will convert stylus coordinates into full dual-screen window space so shared mouse/pointer consumers can hit bottom-screen interactables. The city DS scaffold will stop attaching generic return-to-menu logic directly to the button and instead attach one DS-specific controller that owns both `B` and touch return exactly once per scene lifetime.

**Tech Stack:** C#, shared helengine runtime components, city rendering generator code, Nintendo DS native input backend, xUnit source-audit/editor tests, DS builder/runtime build flow.

---

## File Structure

- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.cpp`
  - Apply the bottom-screen Y offset when publishing stylus mouse/pointer coordinates.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.hpp`
  - Add any DS screen-offset constants or state declarations needed by the backend change.
- Create: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsReturnOverlayComponent.cs`
  - Own DS companion-scene return behavior from one place.
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
  - Stop attaching generic return logic to the button and attach the DS overlay controller instead.
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`
  - Remove the temporary DS-specific input-family split if it is no longer needed once DS ownership is centralized.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\DemoDiscReturnToMenuRuntimeComponent.cs`
  - Keep the shared runtime component aligned with the authored component contract, or remove temporary DS-specific gating if now redundant.
- Create: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs`
  - Assert the DS backend publishes bottom-screen pointer coordinates in full window space.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`
  - Assert the DS scaffold uses the DS overlay controller and no longer attaches the generic return component to the button.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
  - Cover single-owner DS return behavior and repeated `B` / click return requests.

### Task 1: Add the failing DS input backend audit

**Files:**
- Create: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs`

- [ ] **Step 1: Write the failing source-audit test**

```csharp
namespace helengine.ds.builder.tests {
    /// <summary>
    /// Verifies the DS input backend maps bottom-screen stylus coordinates into dual-screen window space.
    /// </summary>
    public sealed class NintendoDsInputBackendSourceAuditTests {
        /// <summary>
        /// Verifies the DS backend offsets stylus Y by the bottom-screen origin before publishing mouse state.
        /// </summary>
        [Fact]
        public void ReadNintendoDsInputBackendSource_OffsetsStylusYIntoBottomScreenWindowSpace() {
            string source = File.ReadAllText(
                Path.Combine(
                    TestPathHelper.ResolveRepositoryRoot(),
                    "src",
                    "platform",
                    "ds",
                    "NintendoDsInputBackend.cpp"));

            Assert.Contains("ScreenHeight", source, StringComparison.Ordinal);
            Assert.Contains("stylusWindowY", source, StringComparison.Ordinal);
            Assert.Contains("stylusPosition.py + ScreenHeight", source, StringComparison.Ordinal);
            Assert.Contains("frame.Mouse = MouseState(", source, StringComparison.Ordinal);
            Assert.Contains("pointerState.Y = stylusWindowY;", source, StringComparison.Ordinal);
        }
    }
}
```

- [ ] **Step 2: Run the new test to verify it fails**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" -v minimal
```

Expected: FAIL because `NintendoDsInputBackend.cpp` does not yet contain `stylusWindowY` or the `+ ScreenHeight` offset.

- [ ] **Step 3: Commit the failing test**

```powershell
git -C C:\dev\helworks\helengine-ds add builder.tests\NintendoDsInputBackendSourceAuditTests.cs
git -C C:\dev\helworks\helengine-ds commit -m "Add failing DS bottom-screen input audit"
```

### Task 2: Fix DS bottom-screen stylus coordinates

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.hpp`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsInputBackendSourceAuditTests.cs`

- [ ] **Step 1: Add the DS bottom-screen window-space constants**

```cpp
namespace {
    /// Stores the raw Nintendo DS hardware bit used by libnds to report an active stylus press.
    constexpr uint32_t NintendoDsTouchKeyMask = (1u << 14);

    /// Stores the logical Nintendo DS screen width used by the shared input contracts.
    constexpr int NintendoDsScreenWidth = 256;

    /// Stores the logical Nintendo DS screen height used by the stacked dual-screen window layout.
    constexpr int NintendoDsScreenHeight = 192;
}
```

- [ ] **Step 2: Publish stylus coordinates in full dual-screen window space**

```cpp
int stylusScreenX = HasPreviousStylusPosition ? PreviousStylusX : 0;
int stylusScreenY = HasPreviousStylusPosition ? PreviousStylusY : 0;
int stylusWindowX = stylusScreenX;
int stylusWindowY = HasPreviousStylusPosition ? PreviousStylusY + NintendoDsScreenHeight : NintendoDsScreenHeight;
int stylusDeltaX = 0;
int stylusDeltaY = 0;
if (stylusIsDown) {
    stylusScreenX = stylusPosition.px;
    stylusScreenY = stylusPosition.py;
    stylusWindowX = stylusScreenX;
    stylusWindowY = stylusScreenY + NintendoDsScreenHeight;
    if (HasPreviousStylusPosition && PreviousStylusPressed) {
        stylusDeltaX = stylusScreenX - PreviousStylusX;
        stylusDeltaY = stylusScreenY - PreviousStylusY;
    }

    PreviousStylusX = stylusScreenX;
    PreviousStylusY = stylusScreenY;
    HasPreviousStylusPosition = true;
}
```

- [ ] **Step 3: Route mouse and pointer state through the window-space coordinates**

```cpp
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
```

- [ ] **Step 4: Run the focused DS input audit**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" -v minimal
```

Expected: PASS.

- [ ] **Step 5: Commit the DS input backend fix**

```powershell
git -C C:\dev\helworks\helengine-ds add src\platform\ds\NintendoDsInputBackend.cpp src\platform\ds\NintendoDsInputBackend.hpp builder.tests\NintendoDsInputBackendSourceAuditTests.cs
git -C C:\dev\helworks\helengine-ds commit -m "Fix DS bottom-screen pointer coordinates"
```

### Task 3: Add the failing DS scaffold ownership tests

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Add the failing scaffold source-audit assertions**

```csharp
[Fact]
public void ReadCityNintendoDsRenderingSceneScaffoldFactorySource_UsesDedicatedDsReturnOverlayComponent() {
    string source = File.ReadAllText(
        Path.Combine(
            TestPathHelper.ResolveHelengineRoot(),
            "..",
            "..",
            "helprojs",
            "city",
            "assets",
            "codebase",
            "rendering.tools",
            "NintendoDsRenderingSceneScaffoldFactory.cs"));

    Assert.Contains("new NintendoDsReturnOverlayComponent()", source, StringComparison.Ordinal);
    Assert.DoesNotContain("new DemoDiscReturnToMenuComponent", source, StringComparison.Ordinal);
    Assert.Contains("RemoveReturnToMenuComponents(entity);", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Add the failing scene-map behavior test for one-shot DS return ownership**

```csharp
[Fact]
public void Update_WhenNintendoDsReturnOverlayReceivesRepeatedBackInput_LoadsMappedSceneOnlyOnce() {
    SceneMapComponent sceneMapComponent = new SceneMapComponent();
    sceneMapComponent.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");

    NintendoDsReturnOverlayComponent component = new NintendoDsReturnOverlayComponent();
    // test setup attaches scene map, overlay controller, and fake input frames
    // then asserts one SceneManager.LoadScene request for DemoDiscMainMenuDs
}
```

- [ ] **Step 3: Run the focused editor tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests.ReadCityNintendoDsRenderingSceneScaffoldFactorySource_UsesDedicatedDsReturnOverlayComponent|FullyQualifiedName~SceneMapServiceTests.Update_WhenNintendoDsReturnOverlayReceivesRepeatedBackInput_LoadsMappedSceneOnlyOnce" -v minimal
```

Expected: FAIL because the DS scaffold still attaches `DemoDiscReturnToMenuComponent` directly and no dedicated DS overlay controller exists yet.

- [ ] **Step 4: Commit the failing ownership tests**

```powershell
git -C C:\dev\helworks\helengine add engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "Add failing DS return overlay ownership tests"
```

### Task 4: Implement the DS-specific return overlay controller

**Files:**
- Create: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsReturnOverlayComponent.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Create the DS overlay controller with single-owner return behavior**

```csharp
namespace city.rendering.tools {
    /// <summary>
    /// Owns Nintendo DS companion-scene return behavior from the bottom overlay.
    /// </summary>
    public sealed class NintendoDsReturnOverlayComponent : UpdateComponent {
        /// <summary>
        /// Logical main menu scene id resolved through the active scene map.
        /// </summary>
        public const string MainMenuSceneId = "DemoDiscMainMenu";

        /// <summary>
        /// Interactable bound to the authored DS bottom-screen back button host.
        /// </summary>
        InteractableComponent BoundInteractable;

        /// <summary>
        /// Tracks whether the active pointer press began inside the bound interactable.
        /// </summary>
        bool PointerPressStartedInside;

        /// <summary>
        /// Tracks whether this component already requested the return transition.
        /// </summary>
        bool SceneLoadWasRequested;
    }
}
```

- [ ] **Step 2: Implement one-time `B` and pointer return handling**

```csharp
public override void Update() {
    TryBindInteractable();

    InputSystem inputSystem = Core.Instance.Input;
    bool wasReturnPressed = inputSystem.WasGamepadButtonPressed(0, InputGamepadButton.East);
    if (wasReturnPressed) {
        LoadResolvedMainMenuScene();
    }
}

void HandleCursorEvent(int2 relativePosition, int2 delta, PointerInteraction interaction) {
    if (interaction == PointerInteraction.Press) {
        PointerPressStartedInside = true;
        return;
    }
    if (interaction == PointerInteraction.Release) {
        bool shouldReturnToMenu = PointerPressStartedInside;
        PointerPressStartedInside = false;
        if (shouldReturnToMenu) {
            LoadResolvedMainMenuScene();
        }
        return;
    }
    if (interaction == PointerInteraction.Leave) {
        PointerPressStartedInside = false;
    }
}

void LoadResolvedMainMenuScene() {
    if (SceneLoadWasRequested) {
        return;
    }

    string resolvedSceneId = SceneMapComponent.ResolveSceneId(MainMenuSceneId);
    SceneLoadWasRequested = true;
    Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single);
}
```

- [ ] **Step 3: Fail fast when the overlay button host is misconfigured**

```csharp
void TryBindInteractable() {
    if (BoundInteractable != null) {
        return;
    } else if (Parent == null || Parent.Components == null) {
        return;
    }

    for (int componentIndex = 0; componentIndex < Parent.Components.Count; componentIndex++) {
        if (Parent.Components[componentIndex] is InteractableComponent interactable) {
            BoundInteractable = interactable;
            BoundInteractable.CursorEvent += HandleCursorEvent;
            return;
        }
    }

    throw new InvalidOperationException("NintendoDsReturnOverlayComponent requires a sibling InteractableComponent.");
}
```

- [ ] **Step 4: Run the focused editor behavior tests**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests.Update_WhenNintendoDsReturnOverlayReceivesRepeatedBackInput_LoadsMappedSceneOnlyOnce" -v minimal
```

Expected: PASS.

- [ ] **Step 5: Commit the DS overlay controller**

```powershell
git -C C:\dev\helprojs\city add assets\codebase\rendering.tools\NintendoDsReturnOverlayComponent.cs
git -C C:\dev\helprojs\city commit -m "Add DS return overlay controller"
```

### Task 5: Move DS scaffold ownership to the new controller

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`

- [ ] **Step 1: Replace button-attached generic return logic with the DS controller**

```csharp
buttonEntity.AddComponent(new InteractableComponent {
    Size = new int2(224, 32)
});
buttonEntity.AddComponent(new NintendoDsReturnOverlayComponent());
buttonEntity.AddComponent(new RoundedRectComponent {
    Size = new int2(224, 32),
    Radius = 0f,
    BorderThickness = 2f,
    FillColor = new byte4(52, 36, 76, 255),
    BorderColor = new byte4(201, 147, 255, 255),
    RenderOrder2D = 230,
    LayerMask = RuntimeLayerMask
});
```

- [ ] **Step 2: Keep stripping authored return components from top-screen scene roots**

```csharp
void ConfigureTopScreenRootRecursive(Entity entity, ref bool assignedPrimaryCamera) {
    if (entity == null) {
        throw new ArgumentNullException(nameof(entity));
    }

    RemoveFpsComponents(entity);
    RemoveReturnToMenuComponents(entity);
    CameraComponent cameraComponent = FindFirstComponent<CameraComponent>(entity);
    // existing camera configuration continues here
}
```

- [ ] **Step 3: Run the DS scaffold source-audit test**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests.ReadCityNintendoDsRenderingSceneScaffoldFactorySource_UsesDedicatedDsReturnOverlayComponent" -v minimal
```

Expected: PASS.

- [ ] **Step 4: Commit the scaffold ownership change**

```powershell
git -C C:\dev\helprojs\city add assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs
git -C C:\dev\helprojs\city commit -m "Move DS companion-scene return ownership to overlay"
```

### Task 6: Remove temporary split return logic from the generic return components

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\DemoDiscReturnToMenuRuntimeComponent.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Collapse the generic return components back to their non-DS contract**

```csharp
public override void ComponentAdded(Entity entity) {
    base.ComponentAdded(entity);
}

public override void Update() {
    InputSystem inputSystem = Core.Instance.Input;
    bool wasReturnPressed = WasKeyboardReturnPressed(inputSystem)
        || WasGamepadReturnPressed(inputSystem);

    if (wasReturnPressed) {
        LoadResolvedMainMenuScene();
    }
}
```

- [ ] **Step 2: Remove pointer binding logic that was only introduced to compensate for DS scaffold ownership**

```csharp
public override void Dispose() {
    base.Dispose();
}

public override void ComponentRemoved(Entity entity) {
    base.ComponentRemoved(entity);
}
```

- [ ] **Step 3: Run the focused scene-map regression slice**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" -v minimal
```

Expected: PASS, including the DS one-shot return coverage and existing mapped-menu regressions.

- [ ] **Step 4: Commit the generic return-component cleanup**

```powershell
git -C C:\dev\helworks\helengine add engine\helengine.core\components\2d\menu\DemoDiscReturnToMenuRuntimeComponent.cs
git -C C:\dev\helprojs\city add assets\codebase\menu\DemoDiscReturnToMenuComponent.cs
git -C C:\dev\helworks\helengine commit -m "Simplify shared return-to-menu component"
git -C C:\dev\helprojs\city commit -m "Simplify authored return-to-menu component"
```

### Task 7: End-to-end verification on DS

**Files:**
- Verify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsInputBackend.cpp`
- Verify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsReturnOverlayComponent.cs`
- Verify: `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

- [ ] **Step 1: Run the full focused automated verification slice**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" -v minimal
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests|FullyQualifiedName~SceneMapServiceTests" -v minimal
```

Expected: PASS.

- [ ] **Step 2: Build the fresh DS ROM**

Run:

```powershell
dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath=C:\dev\helworks\helengine\.codex-build-ds-editor-font\bin\ -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: build succeeds and writes `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`.

- [ ] **Step 3: Relaunch melonDS on the fresh ROM**

Run:

```powershell
Get-Process melonDS -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'
```

Expected: melonDS opens the latest ROM.

- [ ] **Step 4: Perform the manual smoke check**

Check:

```text
1. Open cube_test from the DS menu.
2. Tap the bottom-screen Back button once.
3. Confirm it returns to the DS main menu.
4. Press B repeatedly in cube_test and confirm no "already loaded" exception appears.
5. Re-enter cube_test and confirm the same behavior still holds.
```

- [ ] **Step 5: Commit any final verification-driven adjustments**

```powershell
git -C C:\dev\helworks\helengine-ds status --short
git -C C:\dev\helworks\helengine status --short
git -C C:\dev\helprojs\city status --short
```

Commit only if code changed during smoke-fix follow-up. If no code changed, skip this commit step.
