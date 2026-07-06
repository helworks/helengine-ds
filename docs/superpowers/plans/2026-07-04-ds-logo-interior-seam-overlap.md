# DS Logo Interior Seam Overlap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the thin affine interior seam on the DS logo by snapping tile placement inward from the composed-sprite center.

**Architecture:** Keep the current quantized affine matrix path and shared composed-sprite basis. Replace nearest rounding on per-tile screen-space offsets with center-based inward overlap rounding so adjacent tiles bias toward each other rather than occasionally separating.

**Tech Stack:** C++17, libnds affine OBJ rendering, xUnit source-audit tests, PowerShell build scripts, melonDS

---

## File Map

- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Tighten the focused affine placement audit to require center-based inward overlap rounding.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Declare one helper for center-biased fixed-point offset rounding.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Replace per-tile nearest relative-offset rounding with center-biased inward overlap rounding.

## Task 1: Lock the Seam-Overlap Contract

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Update the focused affine audit**

Require the DS renderer to:

- snap the composed-sprite center once
- resolve tile offsets relative to that center
- round those relative offsets toward zero so tiles bias inward

- [ ] **Step 2: Run the focused test and confirm it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~Source_whenResolvingAffineSpritePlacement_floorsReciprocalScaleAndUsesSharedSnappedOrigin"
```

Expected:

- FAIL

## Task 2: Implement Center-Based Inward Overlap

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`

- [ ] **Step 1: Add one helper for center-biased fixed-point rounding**

Add a helper that rounds screen-space relative offsets toward zero.

- [ ] **Step 2: Snap the composed-sprite center once**

Keep one common snapped basis for all affine logo tiles.

- [ ] **Step 3: Place tiles from center-relative inward offsets**

Use the snapped center plus inward-biased relative offsets instead of per-tile nearest rounding.

- [ ] **Step 4: Run the focused test and confirm it passes**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~Source_whenResolvingAffineSpritePlacement_floorsReciprocalScaleAndUsesSharedSnappedOrigin"
```

Expected:

- PASS

## Task 3: Build and Launch

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
- the interior seam is reduced or gone
