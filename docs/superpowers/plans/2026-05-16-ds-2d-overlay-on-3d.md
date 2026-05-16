# DS 2D Overlay On 3D Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Allow Nintendo DS to render 2D camera queues as an overlay on the same physical screen that owns the hardware 3D pass, with automatic top-first screen ownership.

**Architecture:** Keep the shared engine queue model unchanged and implement the feature entirely in the DS runtime layer. `NintendoDsRenderManager3D` becomes the per-frame hardware 3D screen selector, `NintendoDsRenderManager2D` becomes aware of which screen currently owns 3D so it can present overlay versus full-screen 2D, and `NintendoDsBootHost` stops hardcoding a permanent menu-versus-3D startup mode.

**Tech Stack:** C++, libnds video/display APIs, existing DS builder source audits, `rtk dotnet test`, `rtk dotnet run`

---

## File Map

- Modify: `src/platform/ds/NintendoDsRenderManager3D.hpp`
  - Add the public/private surface for per-frame hardware 3D screen ownership and queue inspection.
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`
  - Implement top-first hardware 3D screen selection, per-screen queue detection, 3D screen configuration, and mixed 2D/3D draw ordering.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  - Add DS screen-target awareness so 2D presentation can distinguish full-screen 2D versus overlay-on-3D presentation.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  - Implement overlay presentation on the active 3D screen and preserve 2D-only presentation on the other screen.
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
  - Remove startup-scene-owned long-lived render-mode policy and leave render-mode choice to the per-frame DS renderer path.
- Create: `src/platform/ds/NintendoDsScreenTarget.hpp`
  - Shared DS-native enum for `None`, `Top`, and `Bottom`.
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  - Lock the new 2D overlay presentation contract.
- Modify: `builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs`
  - Lock top-first hardware 3D screen ownership and mixed draw ordering.
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
  - Lock removal of permanent startup-scene render-mode ownership.
- Test: `builder.tests/helengine.ds.builder.tests.csproj`
  - Run focused DS source audits after each implementation slice.

### Task 1: Add Failing DS Source Audits For Mixed 2D And 3D Presentation

**Files:**
- Create: `src/platform/ds/NintendoDsScreenTarget.hpp`
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs`
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing render-manager 3D audit**

```csharp
[Fact]
public void Source_whenBothScreensContain3d_topScreenWinsAnd2dPresentationStillRunsForBothScreens() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
    string headerSource = File.ReadAllText(headerPath);
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
    Assert.Contains("NintendoDsScreenTarget ResolveHardware3DScreenTarget", headerSource, StringComparison.Ordinal);
    Assert.Contains("bool topScreenHas3D = false;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("bool bottomScreenHas3D = false;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("if (topScreenHas3D) {", sourceCode, StringComparison.Ordinal);
    Assert.Contains("return NintendoDsScreenTarget::Top;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("if (bottomScreenHas3D) {", sourceCode, StringComparison.Ordinal);
    Assert.Contains("return NintendoDsScreenTarget::Bottom;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Write the failing render-manager 2D audit**

```csharp
[Fact]
public void Source_whenOneScreenOwnsHardware3d_presents2dAsOverlayOnThatScreenAnd2dOnlyOnTheOther() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string headerSource = File.ReadAllText(headerPath);
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
    Assert.Contains("void SetHardware3DScreenTarget(NintendoDsScreenTarget target);", headerSource, StringComparison.Ordinal);
    Assert.Contains("NintendoDsScreenTarget Hardware3DScreenTarget;", headerSource, StringComparison.Ordinal);
    Assert.Contains("void NintendoDsRenderManager2D::SetHardware3DScreenTarget(NintendoDsScreenTarget target)", sourceCode, StringComparison.Ordinal);
    Assert.Contains("Hardware3DScreenTarget = target;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("if (Hardware3DScreenTarget == NintendoDsScreenTarget::Top)", sourceCode, StringComparison.Ordinal);
    Assert.Contains("if (Hardware3DScreenTarget == NintendoDsScreenTarget::Bottom)", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 3: Write the failing boot-host audit**

```csharp
[Fact]
public void Source_whenGeneratedCoreRuns_bootHostNoLongerOwnsPermanentMenuVersus3dScreenPolicy() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.DoesNotContain("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("PrepareMainScreenForMenu2D();", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("PrepareMainScreenFor3D();", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("PrepareBottomScreenForMenuProfilingConsole();", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 4: Run the focused source audits to verify they fail**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'
rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests|FullyQualifiedName~NintendoDsBootHostSourceAuditTests" --no-restore -v minimal
```

Expected:

- FAIL because `NintendoDsScreenTarget` does not exist yet
- FAIL because `SetHardware3DScreenTarget(...)` does not exist yet
- FAIL because boot host still owns permanent menu-versus-3D mode setup

- [ ] **Step 5: Commit the failing tests**

```powershell
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs builder.tests/NintendoDsBootHostSourceAuditTests.cs
git commit -m "test: define DS mixed 2d and 3d presentation contract"
```

### Task 2: Add The DS Screen-Target Contract And 3D Screen Ownership Resolution

**Files:**
- Create: `src/platform/ds/NintendoDsScreenTarget.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add the shared DS screen-target enum**

```cpp
#pragma once

namespace helengine::ds {
    /// Identifies which Nintendo DS physical screen currently owns one renderer pass.
    enum class NintendoDsScreenTarget {
        None = 0,
        Top = 1,
        Bottom = 2
    };
}
```

- [ ] **Step 2: Extend the 3D renderer header with ownership-resolution seams**

```cpp
#include "platform/ds/NintendoDsScreenTarget.hpp"

NintendoDsScreenTarget ResolveHardware3DScreenTarget(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D);
void AccumulateCameraScreenQueues(ICamera* camera, bool& topScreenHas3D, bool& bottomScreenHas3D) const;
void ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen);
```

- [ ] **Step 3: Implement top-first hardware 3D target resolution**

```cpp
NintendoDsScreenTarget NintendoDsRenderManager3D::ResolveHardware3DScreenTarget(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D) {
    bool topScreenHas3D = false;
    bool bottomScreenHas3D = false;

    for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++) {
        ICamera* camera = (*cameras)[cameraIndex];
        if (camera == nullptr) {
            continue;
        }

        renderManager2D->DrawCamera(camera);
        AccumulateCameraScreenQueues(camera, topScreenHas3D, bottomScreenHas3D);
    }

    if (topScreenHas3D) {
        return NintendoDsScreenTarget::Top;
    } else if (bottomScreenHas3D) {
        return NintendoDsScreenTarget::Bottom;
    }

    return NintendoDsScreenTarget::None;
}
```

- [ ] **Step 4: Implement hardware main-screen routing for top versus bottom 3D**

```cpp
void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen) {
    if (targetScreen == NintendoDsScreenTarget::Bottom) {
        lcdMainOnBottom();
    } else {
        lcdMainOnTop();
    }

    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_BG0_ACTIVE);
}
```

- [ ] **Step 5: Run the focused source audits to verify this slice passes**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'
rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests" --no-restore -v minimal
```

Expected:

- PASS for the new top-first 3D ownership assertions
- remaining failures still limited to 2D overlay and boot-host cleanup

- [ ] **Step 6: Commit the 3D ownership slice**

```powershell
git add src/platform/ds/NintendoDsScreenTarget.hpp src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp
git commit -m "feat: add DS hardware 3d screen ownership"
```

### Task 3: Teach The 2D Renderer To Present Full 2D Or Overlay On The Active 3D Screen

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add the 2D renderer’s per-frame hardware-3D ownership state**

```cpp
#include "platform/ds/NintendoDsScreenTarget.hpp"

void SetHardware3DScreenTarget(NintendoDsScreenTarget target);
NintendoDsScreenTarget Hardware3DScreenTarget;
bool ActiveViewportTargetsBottomScreen;
```

- [ ] **Step 2: Reset and assign the per-frame 3D ownership state**

```cpp
void NintendoDsRenderManager2D::BeginFrame() {
    Hardware3DScreenTarget = NintendoDsScreenTarget::None;
    ActiveViewportTargetsBottomScreen = false;
    TopScreenClearedThisFrame = false;
    BottomScreenClearedThisFrame = false;
}

void NintendoDsRenderManager2D::SetHardware3DScreenTarget(NintendoDsScreenTarget target) {
    Hardware3DScreenTarget = target;
}
```

- [ ] **Step 3: Present overlay versus full-screen 2D based on the active hardware 3D screen**

```cpp
void NintendoDsRenderManager2D::PresentFrame() {
    if (Hardware3DScreenTarget == NintendoDsScreenTarget::Top) {
        dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
    } else if (Hardware3DScreenTarget == NintendoDsScreenTarget::Bottom) {
        dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
    } else {
        dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
    }

    if (BottomScreenPresentationEnabled) {
        dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM_SUB(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
    }
}
```

- [ ] **Step 4: Update the 3D draw path to set 2D overlay ownership before 2D presentation**

```cpp
NintendoDsScreenTarget hardware3DScreenTarget = ResolveHardware3DScreenTarget(cameras, renderManager2D);
renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);

if (hardware3DScreenTarget != NintendoDsScreenTarget::None) {
    ConfigureHardware3DTarget(hardware3DScreenTarget);
    DrawSelectedScreenRenderQueue(hardware3DScreenTarget, cameras);
}

renderManager2D->PresentFrame();
```

- [ ] **Step 5: Run the focused DS renderer audits**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'
rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests" --no-restore -v minimal
```

Expected:

- PASS for the new 2D overlay presentation assertions
- no regressions in the existing DS 2D and 3D source audits

- [ ] **Step 6: Commit the mixed presentation slice**

```powershell
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp src/platform/ds/NintendoDsRenderManager3D.cpp
git commit -m "feat: overlay DS 2d on the active 3d screen"
```

### Task 4: Remove Boot-Time Screen-Mode Ownership And Verify Multibuild Runtime Transitions

**Files:**
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`
- Test: `C:\tmp\helworks\helengine-ds-city-cube-project\city\project.heproj`

- [ ] **Step 1: Remove permanent startup-scene mode selection from the boot host**

```cpp
void NintendoDsBootHost::RunCheckpointedStartup() {
    RecordBootStatus("[helengine-ds] generated core startup begin");
    PaintCheckpoint(RGB15(0, 31, 0) | BIT(15), RGB15(0, 31, 0) | BIT(15));
    InitializeCore();
    RecordBootStatus("[helengine-ds] generated core initialized");
    PaintCheckpoint(RGB15(31, 31, 0) | BIT(15), RGB15(31, 31, 0) | BIT(15));
    LoadStartupScene();
    RecordBootStatus("[helengine-ds] startup scene load finished");
    PaintCheckpoint(RGB15(0, 31, 31) | BIT(15), RGB15(0, 31, 31) | BIT(15));
    RecordBootStatus("[helengine-ds] entering main loop");
    RunMainLoop();
}
```

- [ ] **Step 2: Keep the diagnostics console narrow and renderer-independent**

```cpp
void NintendoDsBootHost::RunMainLoop() {
    int32_t frameIndex = 0;
    uint32_t previousVBlankCount = VBlankCount;
    while (true) {
        swiWaitForVBlank();
        uint32_t currentVBlankCount = VBlankCount;
        uint32_t elapsedVBlanks = currentVBlankCount > previousVBlankCount ? currentVBlankCount - previousVBlankCount : 1;
        previousVBlankCount = currentVBlankCount;
        double elapsedSeconds = static_cast<double>(elapsedVBlanks) * NintendoDsFrameDeltaSeconds;
        EngineCore->Update(elapsedSeconds);
        EngineCore->Draw();
        frameIndex++;
    }
}
```

- [ ] **Step 3: Run the DS boot-host source audit**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'
rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests" --no-restore -v minimal
```

Expected:

- PASS with no remaining startup-scene-owned permanent render-mode setup

- [ ] **Step 4: Build the DS multibuild ROM end to end**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'
rtk dotnet run --project 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\helengine.ui\helengine.editor.app\helengine.editor.app.csproj' -- --project 'C:\tmp\helworks\helengine-ds-city-cube-project\city\project.heproj' --build ds --output 'C:\tmp\helworks\helengine-ds-city-cube-project\output\ds'
```

Expected:

- `Build completed for platform 'ds': C:\tmp\helworks\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 5: Launch and validate the mixed presentation ROM**

Run:

```powershell
Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList @('C:\tmp\helworks\helengine-ds-city-cube-project\output\ds\helengine_ds.nds')
```

Expected:

- menu multibuild still boots
- scene transitions no longer throw the `Core scene manager must be initialized before runtime menu scene loading can occur` exception
- at least one DS scene can show 3D plus 2D overlay on the same physical screen

- [ ] **Step 6: Commit the boot-host cleanup and verification result**

```powershell
git add src/platform/ds/NintendoDsBootHost.cpp builder.tests/NintendoDsBootHostSourceAuditTests.cs
git commit -m "feat: move DS mixed presentation policy into the render loop"
```

## Plan Review

- Spec coverage:
  - hardware top-first 3D screen ownership: Task 2
  - 2D overlay on the active 3D screen: Task 3
  - removal of boot-time permanent screen-mode policy: Task 4
  - preservation of shared authored scene semantics: Tasks 2-4 stay DS-only
  - build and runtime validation: Task 4
- Placeholder scan:
  - no `TODO`, `TBD`, or “handle appropriately” language remains
  - every code-touching task includes concrete snippets
  - every verification step includes a concrete command and expected result
- Type consistency:
  - `NintendoDsScreenTarget` is the single DS screen-owner enum across tasks
  - `SetHardware3DScreenTarget(...)` is the 2D renderer ownership seam across tasks
  - `ResolveHardware3DScreenTarget(...)` is the 3D renderer ownership seam across tasks
