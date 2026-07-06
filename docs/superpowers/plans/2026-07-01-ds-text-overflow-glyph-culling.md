# DS Text Overflow Glyph Culling Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the Nintendo DS text renderer skip any glyph whose rendered quad would extend outside the text bounds, instead of partially clipping it.

**Architecture:** Keep shared engine text layout unchanged and enforce the new rule only in the DS renderer. Add one glyph-bounds rejection helper in `NintendoDsRenderManager2D`, apply it at final glyph emission time, and cover the behavior with a DS source-audit test plus a minimal build check.

**Tech Stack:** C++, libnds DS renderer code, xUnit source-audit tests, PowerShell build script

---

## File Map

- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Adds one new source-audit test for glyph-level overflow rejection in the DS text renderer.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Declares one focused helper that decides whether a glyph quad is fully inside the text bounds.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Implements the helper and uses it before final glyph emission in the DS raster text path.

### Task 1: Add the Failing DS Text Overflow Audit

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing source-audit test**

Add a new test to `NintendoDsRenderManager2DSourceAuditTests` that asserts:

- `NintendoDsRenderManager2D.hpp` declares a helper with a name like `ShouldRenderGlyphWithinBounds(...) const`
- `NintendoDsRenderManager2D.cpp` computes glyph bounds before `RasterTexturedQuad(...)`
- `NintendoDsRenderManager2D.cpp` skips overflowing glyphs with a guard before `RasterTexturedQuad(...)`

The audit should target the `RasterText(ITextDrawable2D* text)` path because that is where glyph quads are currently emitted.

- [ ] **Step 2: Run the new test to verify it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests
```

Expected:

- FAIL
- Failure should specifically show the new glyph-overflow guard/helper strings are missing

- [ ] **Step 3: Commit the red test**

```powershell
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: add ds text overflow glyph culling audit"
```

### Task 2: Implement Glyph-Level Overflow Rejection in the DS Renderer

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Declare one focused glyph-bounds helper**

In `NintendoDsRenderManager2D.hpp`, add one private helper declaration with one clear responsibility:

- Accept the effective text bounds and the current glyph quad
- Return `true` only when the glyph is fully inside the bounds

Keep the helper local to the DS renderer. Do not add any shared-engine text API.

- [ ] **Step 2: Implement the helper in the DS renderer**

In `NintendoDsRenderManager2D.cpp`, implement the helper so that:

- A glyph is renderable only if all four edges are within bounds
- Any glyph crossing left, right, top, or bottom bounds returns `false`
- The helper performs no clipping or mutation

- [ ] **Step 3: Apply the helper before final glyph emission**

In `RasterText(ITextDrawable2D* text)`:

- Keep existing glyph layout math intact
- After computing `glyphX`, `glyphY`, `glyphWidth`, and `glyphHeight`, compute the glyph rectangle against the text bounds
- If the glyph overflows, skip `RasterTexturedQuad(...)` for that glyph
- Continue advancing layout so later glyphs still follow the existing text flow

Important:

- Do not change engine-wide text layout
- Do not truncate the whole string
- Do not partially clip the glyph

- [ ] **Step 4: Run the source-audit test to verify it passes**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests
```

Expected:

- PASS

- [ ] **Step 5: Commit the DS renderer implementation**

```powershell
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "fix: skip overflowing ds text glyphs"
```

### Task 3: Run the Smallest End-to-End Verification

**Files:**
- Verify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Verify build output: `C:\dev\helprojs\city\output\ds\helengine_ds.nds`

- [ ] **Step 1: Re-run the focused tests**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests
```

Expected:

- PASS

- [ ] **Step 2: Build the DS project**

Run:

```powershell
rtk proxy powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform ds -Output C:\dev\helprojs\city\output\ds
```

Expected:

- Build completed for platform `ds`

- [ ] **Step 3: Launch the fresh ROM**

Run:

```powershell
rtk proxy powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\city\output\ds\helengine_ds.nds
```

Expected:

- melonDS launches the rebuilt ROM

- [ ] **Step 4: Manual verification**

Verify in the DS build:

- A glyph fully inside the text bounds still renders
- A glyph that would overflow is fully absent
- No partially clipped glyph remains visible
- Other glyphs in the same text element still render

- [ ] **Step 5: Commit any final verification-only adjustments if needed**

```powershell
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "test: verify ds text overflow culling"
```
