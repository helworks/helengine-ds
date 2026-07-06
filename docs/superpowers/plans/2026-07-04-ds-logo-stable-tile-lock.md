# DS Logo Stable Tile Lock Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reduce the tiny DS logo affine shimmer by making multi-tile sprite placement derive from one shared snapped origin each frame.

**Architecture:** Keep the existing DS affine matrix path and animation data unchanged. Change only the renderer’s affine tile placement so one snapped composed-sprite origin is computed first, then every tile is placed from that common basis with deterministic integer offsets.

**Tech Stack:** C++17, libnds OBJ affine rendering, xUnit source-audit tests, PowerShell build scripts, melonDS

---

## File Map

- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Declare any small helper needed for shared-origin affine sprite snapping.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Replace independent affine tile rounding with one shared snapped origin plus deterministic tile offsets.
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Lock the renderer onto the shared-origin affine placement path.

## Task 1: Lock the Intended Renderer Contract

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add a focused source-audit expectation for shared-origin affine tile placement**

Assert that the DS renderer source:

- computes one shared affine placement origin before per-tile submission
- places tiles from that shared origin with deterministic offsets
- no longer resolves final DS OBJ coordinates by calling `CeilFixedPointToInteger(drawTileXFixed, FixedPointScale)` and `CeilFixedPointToInteger(drawTileYFixed, FixedPointScale)` directly inside the per-tile loop

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests"
```

Expected:

- FAIL
- the failure should show the renderer still uses independent per-tile fixed-point rounding

## Task 2: Implement Shared-Origin Stable Tile Lock

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add one small helper for stable affine origin snapping if needed**

Keep the helper local to `NintendoDsRenderManager2D` and use it only for shared affine origin resolution.

- [ ] **Step 2: Resolve one shared composed-sprite affine origin before the tile loop**

Use the existing fixed-point center, scale, and rotation data to compute one snapped top-left origin for the full composed sprite.

- [ ] **Step 3: Place each tile from the shared origin**

Replace independent per-tile fixed-point rounding with deterministic integer offsets derived from tile spans and the shared snapped origin.

- [ ] **Step 4: Run the focused source-audit test to verify it passes**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests"
```

Expected:

- PASS

## Task 3: Rebuild and Visually Check the DS ROM

**Files:**
- Verify: `C:\dev\helprojs\city\ds-build\helengine_ds.nds`

- [ ] **Step 1: Build the DS artifact**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform ds -Output C:\dev\helprojs\city\ds-build
```

Expected:

- `Build completed for platform 'ds': C:\dev\helprojs\city\ds-build`

- [ ] **Step 2: Launch in melonDS**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine-ds\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\city\ds-build\helengine_ds.nds
```

Expected:

- melonDS opens the fresh ROM
- the DS logo animation keeps its current motion but with less visible tile shimmer
