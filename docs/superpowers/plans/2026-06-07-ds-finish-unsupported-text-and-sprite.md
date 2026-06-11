# DS Finish Unsupported Text And Sprite Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish the remaining unsupported Nintendo DS text and sprite cases in the active runtime scene while keeping rounded rectangles unsupported and preserving the hardware-only renderer policy.

**Architecture:** Treat the current DS overlay counters and compact labels as the baseline, not as work to redo. Add temporary debug-only reject-reason tracing inside `NintendoDsRenderManager2D`, use the shared build script to identify the live `UT` and `US` reasons in the current scene, then widen only the traced text and sprite gates that were accidentally too strict. If a traced reason is a real DS hardware limitation rather than a missing honest path, keep it unsupported and record that outcome explicitly.

**Tech Stack:** C++, libnds text background and OBJ APIs, xUnit DS source-audit tests, PowerShell build wrappers, melonDS

---

## File Structure

- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Purpose: lock the temporary reject-reason tracing contract and the narrowed text and sprite support gates while work is in flight.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Purpose: declare temporary debug-only reject tracing, per-frame trace guards, and any small helper methods needed to widen honest text or sprite support.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Purpose: implement temporary reject tracing, route every current unsupported text and sprite branch through explicit reasons, and widen only the traced honest support gaps.
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
  Purpose: extend runtime DS OBJ ownership only if the traced sprite work proves the current prepared payload shape is too narrow.
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.cpp`
  Purpose: initialize any new runtime sprite ownership fields safely if Task 5 requires them.
- Modify: `scratch/ds-unsupported-runtime-note.md`
  Purpose: store the live grouped reject reasons captured from the debug ROM before widening support. This file is working state and should stay uncommitted unless the human explicitly asks for it.

## Working Rules

- Rounded rectangles remain unsupported throughout this plan.
- The existing DS overlay contract is already landed: `P1 Tx... S... UT... US...` and `P2 3D... F... P... UR...`. Do not redo that work.
- No software fallback is allowed.
- No bottom-screen logging is allowed.
- Temporary tracing must be debug-only, rate-limited, and removable in one final cleanup task.
- Every support expansion must come from a traced live reject reason, not from guesswork.
- If a traced reason is a true DS hardware limitation, keep the reject path and record that outcome instead of inventing a fake fallback.

## Verification Baseline

Before changing behavior, confirm the current baseline:

- the active runtime scene still shows non-zero `UT` and `US`
- rounded rectangles remain counted separately through `UR`
- the current support gates are still the narrow first-pass rules in `TryDrawHardwareText(...)` and `TryDrawHardwareSprite(...)`
- the current ROM still rebuilds and launches with the shared wrapper

Use these commands during the plan:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsBootHostSourceAuditTests"
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Debug
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Release
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

### Task 1: Lock The Temporary Reject-Reason Trace Contract

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Add failing source-audit expectations for explicit trace helper declarations**

Add assertions that require temporary text and sprite trace entry points plus per-frame trace guards.

```csharp
Assert.Contains("void TraceUnsupportedTextDrawable(ITextDrawable2D* text, const char* reason);", headerSource, StringComparison.Ordinal);
Assert.Contains("void TraceUnsupportedSpriteDrawable(ISpriteDrawable2D* sprite, const char* reason);", headerSource, StringComparison.Ordinal);
Assert.Contains("int32_t UnsupportedTextTraceCountThisFrame;", headerSource, StringComparison.Ordinal);
Assert.Contains("int32_t UnsupportedSpriteTraceCountThisFrame;", headerSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Add failing source-audit expectations for the current reject-reason vocabulary**

Lock the current text and sprite reject gates to named reasons instead of anonymous `return false;` branches.

```csharp
Assert.Contains("\"fontScale\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"wrap\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"alignment\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"color\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"sourceRect\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"bounds\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"charset\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"rotation\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"size\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"texture\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"textureSize\"", sourceCode, StringComparison.Ordinal);
Assert.Contains("\"prepare\"", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 3: Add failing source-audit expectations that tracing stays off the bottom screen**

Require debug-only host-visible tracing rather than console text writes.

```csharp
Assert.Contains("std::fprintf(stderr,", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("iprintf(\"[helengine-ds]", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 4: Run the focused source audit to verify it fails**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: FAIL because the trace helpers and explicit reject-reason strings do not exist yet.

- [ ] **Step 5: Commit the failing audit changes**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: lock DS unsupported reject trace contract"
```

### Task 2: Add Debug-Only Reject Tracing To The Current Renderer Baseline

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Declare temporary trace helpers and per-frame trace guards**

Add separate text and sprite tracing helpers plus tiny per-frame counters.

```cpp
        void TraceUnsupportedTextDrawable(ITextDrawable2D* text, const char* reason);
        void TraceUnsupportedSpriteDrawable(ISpriteDrawable2D* sprite, const char* reason);
        int32_t UnsupportedTextTraceCountThisFrame;
        int32_t UnsupportedSpriteTraceCountThisFrame;
```

- [ ] **Step 2: Initialize and reset the trace guard state**

```cpp
        , UnsupportedTextTraceCountThisFrame(0)
        , UnsupportedSpriteTraceCountThisFrame(0)
```

```cpp
        UnsupportedTextTraceCountThisFrame = 0;
        UnsupportedSpriteTraceCountThisFrame = 0;
```

- [ ] **Step 3: Implement host-visible debug-only tracing with an explicit one-line format**

Keep the trace off the DS console and emit one line per reject up to a small per-frame cap.

```cpp
#if !defined(NDEBUG)
        if (UnsupportedTextTraceCountThisFrame >= 8) {
            return;
        }

        UnsupportedTextTraceCountThisFrame++;
        std::fprintf(stderr, "[helengine-ds] unsupported text reason=%s screen=%s pos=%d,%d\n", safeReason, screenName, anchorX, anchorY);
#endif
```

Use the equivalent format for sprites:

```cpp
std::fprintf(stderr, "[helengine-ds] unsupported sprite reason=%s screen=%s pos=%d,%d\n", safeReason, screenName, anchorX, anchorY);
```

- [ ] **Step 4: Route every current reject branch through explicit reasons before returning `false`**

Text branches should use:

```cpp
TraceUnsupportedTextDrawable(text, "fontScale");
TraceUnsupportedTextDrawable(text, "wrap");
TraceUnsupportedTextDrawable(text, "alignment");
TraceUnsupportedTextDrawable(text, "color");
TraceUnsupportedTextDrawable(text, "sourceRect");
TraceUnsupportedTextDrawable(text, "bounds");
TraceUnsupportedTextDrawable(text, "charset");
```

Sprite branches should use:

```cpp
TraceUnsupportedSpriteDrawable(sprite, "rotation");
TraceUnsupportedSpriteDrawable(sprite, "color");
TraceUnsupportedSpriteDrawable(sprite, "sourceRect");
TraceUnsupportedSpriteDrawable(sprite, "size");
TraceUnsupportedSpriteDrawable(sprite, "texture");
TraceUnsupportedSpriteDrawable(sprite, "textureSize");
TraceUnsupportedSpriteDrawable(sprite, "prepare");
```

- [ ] **Step 5: Run the focused source audit to verify the trace contract passes**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS.

- [ ] **Step 6: Commit the temporary tracing path**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: trace unsupported DS text and sprite reasons"
```

### Task 3: Capture The Live `UT` And `US` Reason Groups

**Files:**
- Modify: `scratch/ds-unsupported-runtime-note.md`
- Test: runtime ROM output only

- [ ] **Step 1: Build the debug DS ROM through the shared wrapper**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Debug
```

Expected: PASS and fresh `.nds` output.

- [ ] **Step 2: Launch the debug ROM**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected: melonDS launches the rebuilt ROM.

- [ ] **Step 3: Group the trace lines into one working note**

Create `scratch/ds-unsupported-runtime-note.md` and summarize the live reject inventory with this exact shape:

```markdown
# DS Unsupported Runtime Note

## Text
- alignment: <count>
- bounds: <count>
- charset: <count>
- color: <count>
- fontScale: <count>
- sourceRect: <count>
- wrap: <count>

## Sprite
- color: <count>
- prepare: <count>
- rotation: <count>
- size: <count>
- sourceRect: <count>
- texture: <count>
- textureSize: <count>

## Outcome
- Active UT reasons: <list>
- Active US reasons: <list>
- Genuine hardware-limit reasons to preserve: <list>
```

- [ ] **Step 4: Stop only when the note explains the current on-screen counters**

Expected: the grouped note accounts for the live `UT` and `US` values in the current scene. Do not widen support until this note is complete.

### Task 4: Eliminate The Active Text Reject Groups

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Modify: `scratch/ds-unsupported-runtime-note.md`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: runtime DS overlay counters

- [ ] **Step 1: Convert the active text reasons from the runtime note into failing source-audit expectations**

For each active text reason that is an honest support gap, add an assertion that locks the new path. For each active text reason that is a real hardware limit, add an assertion that preserves the explicit reject trace.

Use this pattern for supportable alignment work:

```csharp
Assert.Contains("ResolveAlignedConsoleColumn", headerSource, StringComparison.Ordinal);
Assert.Contains("ResolveAlignedConsoleColumn(", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("TraceUnsupportedTextDrawable(text, \"alignment\")", sourceCode, StringComparison.Ordinal);
```

Use this pattern for a preserved hardware-limit reason:

```csharp
Assert.Contains("TraceUnsupportedTextDrawable(text, \"fontScale\")", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 2: Implement only the traced honest text expansions**

The only text reasons that may be widened in this pass are:

- `alignment`: compute the start column for left, center, or right alignment from the visible line width
- `bounds`: clip partially visible text into the console grid instead of rejecting it outright when some characters still fit
- `charset`: normalize `\r`, `\n`, and `\t`, but keep non-ASCII glyphs rejected

Use this alignment helper shape:

```cpp
    int32_t NintendoDsRenderManager2D::ResolveAlignedConsoleColumn(int32_t baseColumn, int32_t visibleLength, int32_t alignment) const {
        if (alignment == 1) {
            return std::clamp(baseColumn - (visibleLength / 2), static_cast<int32_t>(0), (FrameBufferWidth / 8) - 1);
        }

        if (alignment == 2) {
            return std::clamp(baseColumn - visibleLength, static_cast<int32_t>(0), (FrameBufferWidth / 8) - 1);
        }

        return baseColumn;
    }
```

Keep these reasons explicitly unsupported in this pass unless Task 3 proved the gate was wrong:

- `fontScale`
- `color`
- `sourceRect`

- [ ] **Step 3: Run the focused source audit**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS for the widened text contract.

- [ ] **Step 4: Rebuild and launch the release ROM to verify `UT` drops**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Release
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected: `UT` drops to zero or to the clearly recorded hardware-limit floor from the runtime note.

- [ ] **Step 5: Commit the text support expansion**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "feat: widen DS hardware text support"
```

### Task 5: Eliminate The Active Sprite Reject Groups

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.cpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Modify: `scratch/ds-unsupported-runtime-note.md`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: runtime DS overlay counters

- [ ] **Step 1: Convert the active sprite reasons from the runtime note into failing source-audit expectations**

For each active sprite reason that is an honest support gap, add an assertion that locks the new path. For each active sprite reason that is a real hardware limit, add an assertion that preserves the explicit reject trace.

Use this pattern for widened size support:

```csharp
Assert.Contains("IsSupportedHardwareSpriteSize", sourceCode, StringComparison.Ordinal);
Assert.Contains("SpriteSize_16x8", sourceCode, StringComparison.Ordinal);
Assert.Contains("SpriteSize_32x8", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("TraceUnsupportedSpriteDrawable(sprite, \"size\")", sourceCode, StringComparison.Ordinal);
```

Use this pattern for a preserved hardware-limit reason:

```csharp
Assert.Contains("TraceUnsupportedSpriteDrawable(sprite, \"rotation\")", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 2: Implement only the traced honest sprite expansions**

The only sprite reasons that may be widened in this pass are:

- `size`: add more DS OBJ sizes that map directly to real hardware sizes already supported by libnds
- `bounds`: clamp or clip placement only if the sprite remains a normal DS OBJ submission
- `textureSize` or `prepare`: widen the prepared payload shape only if the source texture already matches a real DS OBJ size and format

Keep these reasons explicitly unsupported in this pass unless Task 3 proved the gate was wrong:

- `rotation`
- `color`
- `sourceRect`
- `texture` when the runtime texture is not a `NintendoDsRuntimeTexture2D`

If the traced sprite work needs a wider runtime payload, use this ownership shape:

```cpp
        void* MainHardwareSpriteGraphics;
        void* SubHardwareSpriteGraphics;
```

Do not add speculative caches beyond the exact prepared DS OBJ payload.

- [ ] **Step 3: Run the focused source audit**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS for the widened sprite contract.

- [ ] **Step 4: Rebuild and launch the release ROM to verify `US` drops**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Release
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected: `US` drops to zero or to the clearly recorded hardware-limit floor from the runtime note.

- [ ] **Step 5: Commit the sprite support expansion**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs src/platform/ds/NintendoDsRuntimeTexture2D.hpp src/platform/ds/NintendoDsRuntimeTexture2D.cpp src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "feat: widen DS hardware sprite support"
```

### Task 6: Remove Temporary Tracing And Reconfirm The Final Overlay Counters

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs`
- Test: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
- Test: runtime DS overlay counters

- [ ] **Step 1: Delete the temporary tracing helpers and their audit expectations**

Remove:

```cpp
TraceUnsupportedTextDrawable(...)
TraceUnsupportedSpriteDrawable(...)
UnsupportedTextTraceCountThisFrame
UnsupportedSpriteTraceCountThisFrame
```

Also remove the temporary source-audit assertions that existed only for tracing.

- [ ] **Step 2: Keep the overlay counters and any honest support expansions**

Do not remove `UT`, `US`, or `UR`. Those remain the permanent runtime truth source.

- [ ] **Step 3: Run the focused DS audits**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsBootHostSourceAuditTests"
```

Expected: PASS.

- [ ] **Step 4: Rebuild and launch the final release ROM**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Release
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected: final ROM launches and the bottom-screen counters show the final `UT` and `US` result, with rounded rectangles still represented only through `UR`.

- [ ] **Step 5: Commit the trace cleanup**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp src/platform/ds/NintendoDsRuntimeTexture2D.hpp src/platform/ds/NintendoDsRuntimeTexture2D.cpp
git commit -m "chore: remove temporary DS unsupported tracing"
```

## Self-Review

- Spec coverage: the revised plan starts from the already-landed `UT`/`US`/`UR` overlay baseline, adds temporary reject tracing, captures the live text and sprite reasons, widens only honest hardware support, and removes the temporary tracing at the end.
- Placeholder scan: the previous stale tasks that redid overlay work are gone. The remaining support tasks are constrained to the explicit current reject reasons and define which reasons may or may not be widened in this pass.
- Type consistency: the plan keeps the current file names, DS overlay label meanings, and current renderer method names consistent with the active branch state.
