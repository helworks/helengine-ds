# DS Bottom Console Render Profiler Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move the DS interactive menu to the top screen and use the bottom screen as a native console that reports uncontaminated `NintendoDsRenderManager2D` timing and primitive-count diagnostics.

**Architecture:** Keep all profiling ownership inside the DS renderer and DS boot host. The shared editor-side DS menu generator remaps viewport ownership so the top screen carries the real workload, while `NintendoDsBootHost` owns a native bottom-screen console and prints a throttled renderer snapshot gathered from `NintendoDsRenderManager2D`.

**Tech Stack:** C#/.NET 9 editor tests and scene generation, C++ libnds DS runtime, xUnit source-audit tests, `rtk dotnet` build/test workflow.

---

### Task 1: Remap the DS menu scene for profiling

**Files:**
- Modify: `C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor\managers\menu\NintendoDsDemoMenuSceneAssetFactory.cs`
- Modify: `C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor.tests\managers\menu\EditorMenuSceneRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor.tests\managers\menu\EditorMenuSceneRegenerationServiceTests.cs`

- [ ] **Step 1: Write the failing scene-regeneration test**

Add assertions to `Regenerate_WhenInvokedForNintendoDs_WritesDualScreenMenuScene` so the DS profiling shape is explicit:

```csharp
Assert.Collection(
    sceneAsset.RootEntities,
    entity => Assert.Equal("DemoDiscTopScreenCamera", entity.Name),
    entity => Assert.Equal("DemoDiscBottomScreenCamera", entity.Name));

SceneEntityAsset topCameraEntity = sceneAsset.RootEntities[0];
SceneEntityAsset bottomCameraEntity = sceneAsset.RootEntities[1];

SceneEntityAsset topMenuEntity = Assert.Single(topCameraEntity.Children, entity => entity.Name == "DemoDiscMenuRoot");
Assert.Contains(topMenuEntity.Components, component => component.ComponentTypeId == MenuComponent.SerializedComponentTypeId);
Assert.Contains(topMenuEntity.Components, component => component.ComponentTypeId == "helengine.ViewportComponent, helengine.core");

SceneEntityAsset bottomRootEntity = Assert.Single(bottomCameraEntity.Children, entity => entity.Name == "DemoDiscBottomScreenRoot");
Assert.DoesNotContain(bottomRootEntity.Components, component => component.ComponentTypeId == "helengine.FPSComponent");
Assert.DoesNotContain(FlattenEntityNames(bottomRootEntity), name => name == "DemoDiscOverlayImage");
Assert.DoesNotContain(FlattenEntityNames(bottomRootEntity), name => name.Contains("heading", StringComparison.OrdinalIgnoreCase));
```

- [ ] **Step 2: Run the editor test to verify it fails**

Run:

```powershell
rtk dotnet test 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor.tests\helengine.editor.tests.csproj' --filter "FullyQualifiedName~EditorMenuSceneRegenerationServiceTests.Regenerate_WhenInvokedForNintendoDs_WritesDualScreenMenuScene" --no-restore -v minimal
```

Expected: FAIL because the current generator still places branding/FPS content on the top screen and the menu on the bottom screen.

- [ ] **Step 3: Write the minimal DS menu-scene remap**

Update `NintendoDsDemoMenuSceneAssetFactory.cs` so the DS profiling build shape is:

```csharp
SceneEntityAsset BuildTopCameraEntityAsset(string providerTypeName, MenuDefinition definition) {
    return new SceneEntityAsset {
        Id = AllocateSceneEntityId(),
        Name = "DemoDiscTopScreenCamera",
        LocalPosition = float3.Zero,
        LocalScale = float3.One,
        LocalOrientation = float4.Identity,
        Components = [
            BuildCameraComponentRecord(new float4(0f, 0f, 1f, 1f), definition.BackgroundColor)
        ],
        Children = [
            BuildTopMenuRootEntityAsset(providerTypeName, definition)
        ]
    };
}

SceneEntityAsset BuildBottomCameraEntityAsset(MenuDefinition definition) {
    return new SceneEntityAsset {
        Id = AllocateSceneEntityId(),
        Name = "DemoDiscBottomScreenCamera",
        LocalPosition = float3.Zero,
        LocalScale = float3.One,
        LocalOrientation = float4.Identity,
        Components = [
            BuildCameraComponentRecord(new float4(0f, 1f, 1f, 1f), definition.BackgroundColor)
        ],
        Children = [
            BuildBottomScreenConsoleRootEntityAsset()
        ]
    };
}
```

And introduce a minimal bottom root with no menu visuals:

```csharp
SceneEntityAsset BuildBottomScreenConsoleRootEntityAsset() {
    ViewportComponent viewportComponent = new ViewportComponent {
        BindingMode = ViewportComponent.AncestorCameraBindingMode,
        FixedSize = new int2(DemoMenuNintendoDsLayout.ScreenWidth, DemoMenuNintendoDsLayout.ScreenHeight),
        ScalingMode = ViewportComponent.ReferenceCanvasScalingMode,
        ReferenceWidth = DemoMenuNintendoDsLayout.ScreenWidth,
        ReferenceHeight = DemoMenuNintendoDsLayout.ScreenHeight
    };

    return new SceneEntityAsset {
        Id = AllocateSceneEntityId(),
        Name = "DemoDiscBottomScreenRoot",
        LocalPosition = float3.Zero,
        LocalScale = float3.One,
        LocalOrientation = float4.Identity,
        Components = [
            AutomaticDescriptor.SerializeComponent(viewportComponent, 0, null)
        ],
        Children = Array.Empty<SceneEntityAsset>()
    };
}
```

- [ ] **Step 4: Run the editor test to verify it passes**

Run:

```powershell
rtk dotnet test 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor.tests\helengine.editor.tests.csproj' --filter "FullyQualifiedName~EditorMenuSceneRegenerationServiceTests.Regenerate_WhenInvokedForNintendoDs_WritesDualScreenMenuScene" --no-restore -v minimal
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
git -C 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity' add -- 'engine/helengine.editor/managers/menu/NintendoDsDemoMenuSceneAssetFactory.cs' 'engine/helengine.editor.tests/managers/menu/EditorMenuSceneRegenerationServiceTests.cs'
git -C 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity' commit -m "test: remap DS menu scene for profiling"
```

### Task 2: Add renderer-owned DS 2D profiling snapshots

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\src\platform\ds\NintendoDsRenderManager2D.hpp`
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\src\platform\ds\NintendoDsRenderManager2D.cpp`
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Write the failing DS renderer source audit**

Add a new audit that requires frame-local profiling state and public snapshot access:

```csharp
[Fact]
public void Source_whenProfilingNintendoDs2dRenderer_tracksPerPrimitiveTimingAndCounts() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string headerSource = File.ReadAllText(headerPath);
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("struct NintendoDsRenderManager2DProfileSnapshot", headerSource, StringComparison.Ordinal);
    Assert.Contains("NintendoDsRenderManager2DProfileSnapshot get_ProfileSnapshot() const;", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileTextPrimitiveCount", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileSpritePrimitiveCount", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileRoundedRectPrimitiveCount", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileTextMilliseconds", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileSpriteMilliseconds", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileRoundedRectMilliseconds", headerSource, StringComparison.Ordinal);
    Assert.Contains("ProfileTotalFrameMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("ProfileTextPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("ProfileSpritePrimitiveCount++;", sourceCode, StringComparison.Ordinal);
    Assert.Contains("ProfileRoundedRectPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the renderer source audit to verify it fails**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'; rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests.Source_whenProfilingNintendoDs2dRenderer_tracksPerPrimitiveTimingAndCounts" --no-restore -v minimal
```

Expected: FAIL because the renderer does not expose profiling buckets yet.

- [ ] **Step 3: Write the minimal profiling snapshot implementation**

Add a small public snapshot contract in `NintendoDsRenderManager2D.hpp`:

```cpp
struct NintendoDsRenderManager2DProfileSnapshot {
    double TotalFrameMilliseconds;
    double TextMilliseconds;
    double SpriteMilliseconds;
    double RoundedRectMilliseconds;
    int32_t TextPrimitiveCount;
    int32_t SpritePrimitiveCount;
    int32_t RoundedRectPrimitiveCount;
};
```

Add fields and reset behavior in `BeginFrame()`:

```cpp
ProfileTotalFrameMilliseconds = 0.0;
ProfileTextMilliseconds = 0.0;
ProfileSpriteMilliseconds = 0.0;
ProfileRoundedRectMilliseconds = 0.0;
ProfileTextPrimitiveCount = 0;
ProfileSpritePrimitiveCount = 0;
ProfileRoundedRectPrimitiveCount = 0;
```

Measure the three draw entry points with the smallest change possible:

```cpp
void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text) {
    double beginMilliseconds = timerElapsedMilliseconds();
    RasterText(text);
    ProfileTextMilliseconds += timerElapsedMilliseconds() - beginMilliseconds;
    ProfileTextPrimitiveCount++;
}
```

Apply the same pattern for sprites and rounded rects, and accumulate `ProfileTotalFrameMilliseconds` across `DrawCamera`.

- [ ] **Step 4: Run the renderer source audit to verify it passes**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'; rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests" --no-restore -v minimal
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
git -C 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity' add -- 'src/platform/ds/NintendoDsRenderManager2D.hpp' 'src/platform/ds/NintendoDsRenderManager2D.cpp' 'builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs'
git -C 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity' commit -m "feat: add DS 2D renderer profiling snapshot"
```

### Task 3: Print the profiling snapshot on a native DS bottom-screen console

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\src\platform\ds\NintendoDsBootHost.cpp`
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\NintendoDsBootHostSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\NintendoDsBootHostSourceAuditTests.cs`

- [ ] **Step 1: Write the failing DS boot-host source audit**

Add a source audit that requires native bottom-console output driven by the 2D renderer snapshot:

```csharp
[Fact]
public void Source_whenProfilingDsMenu_rendersMenuOnTopAndWrites2dSnapshotToBottomConsole() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("void NintendoDsBootHost::InitializeStatusConsole()", sourceCode, StringComparison.Ordinal);
    Assert.Contains("EngineRenderManager2D->get_ProfileSnapshot()", sourceCode, StringComparison.Ordinal);
    Assert.Contains("consoleSelect(&StatusConsole);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("consoleClear();", sourceCode, StringComparison.Ordinal);
    Assert.Contains("iprintf(\"2D total", sourceCode, StringComparison.Ordinal);
    Assert.Contains("iprintf(\"Text", sourceCode, StringComparison.Ordinal);
    Assert.Contains("iprintf(\"Sprite", sourceCode, StringComparison.Ordinal);
    Assert.Contains("iprintf(\"Rect", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the boot-host source audit to verify it fails**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'; rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests.Source_whenProfilingDsMenu_rendersMenuOnTopAndWrites2dSnapshotToBottomConsole" --no-restore -v minimal
```

Expected: FAIL because the current boot host still emits FPS-style diagnostics instead of a native console profiler report.

- [ ] **Step 3: Write the minimal boot-host profiler console**

Update `NintendoDsBootHost.cpp` so the bottom-screen console is the profiling sink:

```cpp
void NintendoDsBootHost::EmitBottomConsoleProfileDiagnostic(int32_t frameIndex) {
    if (!StatusConsoleInitialized) {
        throw new InvalidOperationException("Bottom-screen status console must be initialized before profiling output.");
    } else if (EngineRenderManager2D == nullptr) {
        throw new InvalidOperationException("Nintendo DS 2D renderer must exist before profiling output.");
    }

    NintendoDsRenderManager2DProfileSnapshot snapshot = EngineRenderManager2D->get_ProfileSnapshot();
    consoleSelect(&StatusConsole);
    consoleClear();
    iprintf("frame %d\n", frameIndex);
    iprintf("update %.1f\n", EngineCore->get_LastUpdateFramesPerSecond());
    iprintf("2D total %.2f ms\n", snapshot.TotalFrameMilliseconds);
    iprintf("Text %.2f ms  %d\n", snapshot.TextMilliseconds, snapshot.TextPrimitiveCount);
    iprintf("Sprite %.2f ms  %d\n", snapshot.SpriteMilliseconds, snapshot.SpritePrimitiveCount);
    iprintf("Rect %.2f ms  %d\n", snapshot.RoundedRectMilliseconds, snapshot.RoundedRectPrimitiveCount);
}
```

Call it from the main loop at a throttled cadence:

```cpp
if ((frameIndex % 15) == 0) {
    EmitBottomConsoleProfileDiagnostic(frameIndex);
}
```

Remove or bypass the current FPS-overlay-driven timing log path for this profiling configuration so the bottom console becomes the primary visible diagnostic surface.

- [ ] **Step 4: Run the boot-host source audit to verify it passes**

Run:

```powershell
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'; rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests" --no-restore -v minimal
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
git -C 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity' add -- 'src/platform/ds/NintendoDsBootHost.cpp' 'builder.tests/NintendoDsBootHostSourceAuditTests.cs'
git -C 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity' commit -m "feat: add DS bottom-console renderer profiler"
```

### Task 4: Rebuild the DS ROM and verify the profiler view in melonDS

**Files:**
- Modify: `C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor\managers\menu\NintendoDsDemoMenuSceneAssetFactory.cs`
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\src\platform\ds\NintendoDsBootHost.cpp`
- Modify: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\src\platform\ds\NintendoDsRenderManager2D.cpp`
- Test: `C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor.tests\managers\menu\EditorMenuSceneRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\NintendoDsBootHostSourceAuditTests.cs`

- [ ] **Step 1: Run the focused verification suites**

Run:

```powershell
rtk dotnet test 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\engine\helengine.editor.tests\helengine.editor.tests.csproj' --filter "FullyQualifiedName~EditorMenuSceneRegenerationServiceTests.Regenerate_WhenInvokedForNintendoDs_WritesDualScreenMenuScene" --no-restore -v minimal
$env:HELENGINE_ROOT='C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity'; rtk dotnet test 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity\builder.tests\helengine.ds.builder.tests.csproj' --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsBootHostSourceAuditTests" --no-restore -v minimal
```

Expected: PASS on both commands.

- [ ] **Step 2: Rebuild the DS ROM from the paired worktrees**

Run:

```powershell
rtk dotnet run --project 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity\helengine.ui\helengine.editor.app\helengine.editor.app.csproj' -- --project 'C:\tmp\helengine-ds-city-cube-project\city\project.heproj' --build ds --output 'C:\tmp\helworks\helengine-ds-city-cube-project\output\ds'
```

Expected:

```text
Build completed for platform 'ds': C:\tmp\helworks\helengine-ds-city-cube-project\output\ds
```

- [ ] **Step 3: Launch melonDS with the rebuilt ROM**

Run:

```powershell
Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helworks\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'
```

Expected manual result:

```text
Top screen: interactive menu
Bottom screen: native DS console showing frame/update/2D total/Text/Sprite/Rect diagnostics
```

- [ ] **Step 4: Capture the first real profiling reading**

Manual check in `melonDS`:

```text
1. Confirm the bottom screen is native console text, not engine-rendered UI.
2. Read one full profiler sample set.
3. Record which bucket is dominant: Text, Sprite, Rect, or Total frame outside those categories.
```

Expected: one stable reading that identifies the next optimization target.

- [ ] **Step 5: Commit**

```powershell
git -C 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity' add -- 'engine/helengine.editor/managers/menu/NintendoDsDemoMenuSceneAssetFactory.cs' 'engine/helengine.editor.tests/managers/menu/EditorMenuSceneRegenerationServiceTests.cs'
git -C 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity' add -- 'src/platform/ds/NintendoDsBootHost.cpp' 'src/platform/ds/NintendoDsRenderManager2D.cpp' 'src/platform/ds/NintendoDsRenderManager2D.hpp' 'builder.tests/NintendoDsBootHostSourceAuditTests.cs' 'builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs'
git -C 'C:\dev\helworks\helengine\.worktrees\ds-menu-input-parity' commit -m "feat: remap DS menu for profiling"
git -C 'C:\dev\helworks\helengine-ds\.worktrees\ds-menu-input-parity' commit -m "feat: add DS native render profiler"
```
