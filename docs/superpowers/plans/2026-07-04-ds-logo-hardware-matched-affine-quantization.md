# DS Logo Hardware-Matched Affine Quantization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reduce the last small DS logo shimmer by using the quantized DS affine matrix basis for CPU-side multi-tile placement too.

**Architecture:** Keep the current shared reference-tile snap and existing affine matrix submission. Change only the CPU placement basis so quantized angle and inverse-scale values become the source of truth for both placement and hardware submission.

**Tech Stack:** C++17, libnds affine OBJ rendering, xUnit source-audit tests, PowerShell build scripts, melonDS

---

## File Map

- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Tighten the focused affine placement audit to require hardware-matched quantization.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Declare any small helper needed to reconstruct quantized visual angle or scale.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Reconstruct the CPU placement basis from quantized affine values before shared tile locking.

## Task 1: Lock the Quantized Placement Contract

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Update the focused affine audit**

Require the DS renderer to:

- derive visual placement rotation from quantized affine angle
- derive visual placement scale from quantized affine scale values
- keep shared reference-tile locking

- [ ] **Step 2: Run the focused test and confirm it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~Source_whenResolvingAffineSpritePlacement_floorsReciprocalScaleAndUsesSharedSnappedOrigin"
```

Expected:

- FAIL

## Task 2: Implement Hardware-Matched Quantization

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`

- [ ] **Step 1: Add helper declarations if needed**

Add small renderer helpers only if they make the quantized basis explicit and reusable.

- [ ] **Step 2: Reconstruct quantized visual scale and rotation from affine inputs**

Use quantized DS affine angle and inverse-scale values for CPU placement math.

- [ ] **Step 3: Keep shared tile locking on top of the quantized basis**

Do not replace the current shared reference-tile snap path.

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
- the remaining logo shimmer is reduced again
