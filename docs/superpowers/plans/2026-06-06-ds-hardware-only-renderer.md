# DS Hardware-Only Renderer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the Nintendo DS software 2D compositor and placeholder rendering capability lies so the DS backend renders only through real hardware paths, skips unsupported drawables, and shows debug-only magenta hardware markers for unsupported work.

**Architecture:** The DS 2D render manager remains the engine-facing 2D integration point, but it becomes a hardware capability router instead of a CPU compositor. The DS 3D render manager stops coordinating software 2D presentation and rejects unsupported render-target creation explicitly, while the DS boot host stops prewarming caches that no longer exist.

**Tech Stack:** C++ Nintendo DS platform renderer code, libnds hardware APIs, shared helengine runtime interfaces, xUnit source-audit tests in `builder.tests`, git.

---

## File Structure

- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
  - Replace assertions that preserve CPU framebuffers, DMA presentation, cached bitmap text, and visible software 2D state with assertions for the hardware-only contract.
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`
  - Remove expectations that the 3D draw loop calls `renderManager2D->PresentFrame();` and replace them with no-software-coupling audits.
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`
  - Replace the “skip invisible CPU present” contract with a stronger “no software 2D present path exists” contract and add render-target rejection coverage.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.hpp`
  - Remove CPU framebuffer, cache, and software-visibility declarations; add hardware-only unsupported-draw diagnostic declarations and any debug-marker state.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp`
  - Remove CPU composition, CPU `PresentFrame()`, cached bitmap/fallback text, rounded-rectangle rasterization, and software visibility tracking; implement hardware-only drawable routing, per-drawable skip handling, debug logging, and magenta hardware markers.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.hpp`
  - Remove software-2D-present coordination declarations and update `CreateRenderTarget(...)` documentation to reflect explicit unsupported behavior.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
  - Remove `ShouldPresent2DFrame(...)`, remove `renderManager2D->PresentFrame();` call sites, and make `CreateRenderTarget(...)` reject DS off-screen render-target requests.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsBootHost.cpp`
  - Remove startup text-cache prewarm traversal or repurpose it to hardware-only prep if one real hardware path still benefits from eager setup.
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`
  - Update any startup audits that currently assume DS text-cache prewarm behavior survives this renderer policy change.

### Task 1: Replace the 2D source-audit contract with the hardware-only policy

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Rewrite the failing 2D source-audit tests**

```csharp
[Fact]
public void Source_whenDeclaringNintendoDsRenderManager2d_doesNotKeepCpuFramebuffersOrSoftwarePresentation() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string headerSource = File.ReadAllText(headerPath);
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.DoesNotContain("TopCpuFrameBuffer", headerSource, StringComparison.Ordinal);
    Assert.DoesNotContain("BottomCpuFrameBuffer", headerSource, StringComparison.Ordinal);
    Assert.DoesNotContain("void PresentFrame();", headerSource, StringComparison.Ordinal);
    Assert.DoesNotContain("dmaCopyHalfWords(3, TopCpuFrameBuffer.data()", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("dmaCopyHalfWords(3, BottomCpuFrameBuffer.data()", sourceCode, StringComparison.Ordinal);
}

[Fact]
public void Source_whenHandlingUnsupported2dWork_skipsSoftwareFallbackAndDefinesDebugMarkerPath() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string headerSource = File.ReadAllText(headerPath);
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.DoesNotContain("TextBitmapCache", headerSource, StringComparison.Ordinal);
    Assert.DoesNotContain("TextFallbackGlyphCount", headerSource, StringComparison.Ordinal);
    Assert.Contains("void DrawUnsupportedDrawableMarker(", headerSource, StringComparison.Ordinal);
    Assert.Contains("void LogUnsupportedDrawable(", headerSource, StringComparison.Ordinal);
    Assert.Contains("DrawUnsupportedDrawableMarker(", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the 2D source-audit tests to verify they fail against the current software-path code**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests 2>&1 | head -c 4000`
Expected: FAIL because the current DS 2D renderer still declares CPU framebuffers, `PresentFrame()`, and bitmap-cache software paths.

- [ ] **Step 3: Commit the failing-test-only checkpoint**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: replace DS 2D audits with hardware-only contract"
```

### Task 2: Replace the 3D and startup audits that still preserve software 2D behavior

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`

- [ ] **Step 1: Rewrite the failing 3D and boot-host audits**

```csharp
[Fact]
public void Source_whenDrawingNintendoDsFrame_neverCallsSoftware2dPresentation() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.DoesNotContain("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("ShouldPresent2DFrame(", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("get_FrameHasVisibleSoftware2DWork()", sourceCode, StringComparison.Ordinal);
}

[Fact]
public void Source_whenCreatingNintendoDsRenderTarget_rejectsPlaceholderCapabilityLie() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.DoesNotContain("RenderTarget* renderTarget = new RenderTarget();", sourceCode, StringComparison.Ordinal);
    Assert.Contains("throw new InvalidOperationException(", sourceCode, StringComparison.Ordinal);
}

[Fact]
public void Source_whenBootingNintendoDs_doesNotPrewarmRemovedSoftwareTextCaches() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.DoesNotContain("PrewarmEntityTreeTextCaches", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("PrewarmTextDrawable(", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the rewritten 3D/startup source-audit tests to verify they fail**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "NintendoDsRenderManager3DSourceAuditTests|NintendoDsRenderManager3DPerformanceSourceAuditTests|NintendoDsBootHostSourceAuditTests" 2>&1 | head -c 4000`
Expected: FAIL because the current 3D renderer still coordinates software 2D presentation, returns placeholder render targets, and the boot host still prewarms software text caches.

- [ ] **Step 3: Commit the failing-test checkpoint**

```bash
git add builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs builder.tests/NintendoDsBootHostSourceAuditTests.cs
git commit -m "test: audit DS renderer for hardware-only policy"
```

### Task 3: Strip software-path state from `NintendoDsRenderManager2D`

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Remove software-path declarations from the 2D header**

Make these structural edits:

- delete `TopCpuFrameBuffer`, `BottomCpuFrameBuffer`, and `ActiveCpuFrameBuffer`,
- delete `PresentFrame()`,
- delete `get_FrameHasVisibleSoftware2DWork()` and `FrameHasVisibleSoftware2DWork`,
- delete bitmap-cache and rounded-rect cache declarations that exist only for CPU composition,
- delete profiling counters that only measure removed software work,
- add explicit unsupported-draw handling declarations such as:

```cpp
void LogUnsupportedDrawable(const char* category, Entity* parentEntity);
void DrawUnsupportedDrawableMarker(int32_t x, int32_t y, NintendoDsScreenTarget targetBottomScreen);
bool TryDrawHardwareSprite(ISpriteDrawable2D* sprite);
bool TryDrawHardwareText(ITextDrawable2D* text);
```

- [ ] **Step 2: Run the 2D source-audit tests to confirm the header still fails for the missing source changes**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests 2>&1 | head -c 4000`
Expected: FAIL because the source file still contains the CPU render flow and missing unsupported-marker implementation.

- [ ] **Step 3: Remove the CPU compositor and fallback code from the 2D source**

Implement the minimal source changes to satisfy the new header contract:

- remove `PresentFrame()` entirely,
- remove CPU backbuffer clears and CPU framebuffer selection,
- remove cached bitmap text creation and fallback glyph rasterization,
- remove rounded-rectangle CPU rasterization,
- keep `Visit(...)` methods but route each supported drawable through `TryDrawHardware...(...)`,
- if a drawable is unsupported:
  - return without throwing in release,
  - call `LogUnsupportedDrawable(...)` and `DrawUnsupportedDrawableMarker(...)` in debug.

Use a single DS-compatible magenta packed color constant for the marker path, for example:

```cpp
constexpr uint16_t UnsupportedDebugMarkerColor = RGB15(31, 0, 31);
```

- [ ] **Step 4: Run the 2D source-audit tests to verify they pass**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests 2>&1 | head -c 4000`
Expected: PASS.

- [ ] **Step 5: Commit the 2D renderer refactor**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "refactor: remove DS 2D software compositor"
```

### Task 4: Remove software-presentation coupling from `NintendoDsRenderManager3D`

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`

- [ ] **Step 1: Remove software-2D present declarations from the 3D header**

Delete or replace:

- `bool ShouldPresent2DFrame(...) const;`
- any `LastPresent...` metrics that only exist to measure removed software presentation,
- any comments that describe CPU bitmap presentation as part of the DS frame flow.

Update `CreateRenderTarget(...)` comments so they describe explicit unsupported behavior rather than placeholder support.

- [ ] **Step 2: Run the 3D source-audit tests to confirm the source still fails**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "NintendoDsRenderManager3DSourceAuditTests|NintendoDsRenderManager3DPerformanceSourceAuditTests" 2>&1 | head -c 4000`
Expected: FAIL because the source file still contains software-presentation logic and placeholder render-target creation.

- [ ] **Step 3: Remove software-presentation flow from the 3D source**

Implement these minimal source edits:

- delete `ShouldPresent2DFrame(...)`,
- delete all `renderManager2D->PresentFrame();` calls,
- delete logic that inspects `renderManager2D->get_FrameHasVisibleSoftware2DWork()`,
- remove `LastPresentNetByteDelta` / related metrics if they now exist only for the removed path,
- change `CreateRenderTarget(...)` from:

```cpp
RenderTarget* renderTarget = new RenderTarget();
renderTarget->set_Width(width);
renderTarget->set_Height(height);
return renderTarget;
```

to an explicit DS rejection:

```cpp
throw new InvalidOperationException("Nintendo DS render targets are unsupported because this backend only exposes real hardware render paths.");
```

- [ ] **Step 4: Run the 3D source-audit tests to verify they pass**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "NintendoDsRenderManager3DSourceAuditTests|NintendoDsRenderManager3DPerformanceSourceAuditTests" 2>&1 | head -c 4000`
Expected: PASS.

- [ ] **Step 5: Commit the 3D renderer refactor**

```bash
git add src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs
git commit -m "refactor: remove DS software presentation coupling"
```

### Task 5: Remove boot-time prewarm logic tied to deleted caches

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsBootHost.cpp`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`

- [ ] **Step 1: Remove or rewrite the boot-host software-cache audit**

Add or update the audit so it verifies startup does not invoke removed software-cache prewarm helpers:

```csharp
Assert.DoesNotContain("PrewarmEntityTreeTextCaches", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("PrewarmTextDrawable(", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the boot-host audit to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter NintendoDsBootHostSourceAuditTests 2>&1 | head -c 4000`
Expected: FAIL because startup still walks entities to prewarm removed 2D text caches.

- [ ] **Step 3: Remove the boot-host prewarm traversal**

Delete:

- `PrewarmEntityTreeTextCaches(...)`,
- the recursive child traversal that exists only to call `PrewarmTextDrawable(...)`,
- the startup invocation that triggers this prewarm phase before the first visible frame.

If a remaining hardware-only text path still needs one-time setup, move that setup into renderer initialization instead of scene-tree traversal.

- [ ] **Step 4: Run the boot-host audit to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter NintendoDsBootHostSourceAuditTests 2>&1 | head -c 4000`
Expected: PASS.

- [ ] **Step 5: Commit the startup cleanup**

```bash
git add src/platform/ds/NintendoDsBootHost.cpp builder.tests/NintendoDsBootHostSourceAuditTests.cs
git commit -m "refactor: remove DS software text prewarm"
```

### Task 6: Run focused verification for the full hardware-only policy slice

**Files:**
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`

- [ ] **Step 1: Run the focused DS renderer audit suite**

Run: `rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsRenderManager3DPerformanceSourceAuditTests|NintendoDsBootHostSourceAuditTests" 2>&1 | head -c 4000`
Expected: PASS.

- [ ] **Step 2: Run one small DS native build validation if the renderer audits pass cleanly**

Run: `rtk git status --short --branch 2>&1 | head -c 1000`
Expected: Only the intended DS renderer, boot-host, plan, and test changes remain staged or modified.

If an approved DS native build command is available in the current environment, run that smallest existing DS build target after the audit suite passes. Do not expand scope beyond the DS renderer slice.

- [ ] **Step 3: Commit the final verified slice**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp src/platform/ds/NintendoDsBootHost.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs builder.tests/NintendoDsBootHostSourceAuditTests.cs
git commit -m "refactor: enforce hardware-only DS renderer policy"
```
