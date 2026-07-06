# DS Logo Static X-Scale Probe Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the DS menu logo animation with one fixed X-only fractional scale probe so affine scaling can be debugged without animation noise.

**Architecture:** Keep the change localized to the city DS menu-scene authoring path. Update the existing DS menu source audit first, then remove the animation player and author one fixed fractional X scale directly on the DS logo entity before regenerating the menu scene and rebuilding DS.

**Tech Stack:** C#, xUnit source-audit tests, city generated-scene pipeline, PowerShell build scripts, melonDS

---

## File Map

- Modify: `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs`
  Replace the current DS-logo animation expectation with one static X-only fractional scale expectation.
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
  Remove the DS logo animation player and author one fixed fractional X-only scale.
- Create: `docs/superpowers/plans/2026-07-04-ds-logo-static-x-scale-probe.md`
  Records the implementation path for this DS-only debug change.

## Task 1: Write the Failing Source Audit

**Files:**
- Modify: `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Change the DS logo source audit**

Update the existing test so it asserts the DS logo method:

- sets `entity.LocalScale = new float3(0.95f, 1f, 1f);`
- does not contain `new AnimationPlayerComponent`
- does not contain `ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath);`

- [ ] **Step 2: Run the test to verify it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsDemoDiscMenuSourceAuditTests"
```

Expected:

- FAIL
- failure should show the DS logo method still authors the animation player and does not set the fixed scale

## Task 2: Author the Static DS X-Scale Probe

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Remove the DS logo animation player**

Delete the `AnimationPlayerComponent` creation, component add, and clip-reference serialization from `CreateNintendoDsTopScreenLogoEntity(...)`.

- [ ] **Step 2: Add the fixed fractional X-only scale**

Set:

```csharp
entity.LocalScale = new float3(0.95f, 1f, 1f);
```

Keep the existing fixed position and orientation logic unchanged.

- [ ] **Step 3: Run the source-audit test to verify it passes**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsDemoDiscMenuSourceAuditTests"
```

Expected:

- PASS

## Task 3: Regenerate and Verify the Baked Scene

**Files:**
- Verify: `C:\dev\helprojs\city\assets\scenes\DemoDiscMainMenuDs.helen`

- [ ] **Step 1: Regenerate the menu scene**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.regenerate-demo-disc-main-menu
```

Expected:

- `Editor command 'menu.regenerate-demo-disc-main-menu' executed successfully.`

- [ ] **Step 2: Inspect the baked DS scene**

Run:

```powershell
rtk dotnet run --project .\scratch\SceneAssetDump\SceneAssetDump.csproj -- C:\dev\helprojs\city\assets\scenes\DemoDiscMainMenuDs.helen
```

Expected:

- `DemoDiscOverlayImage` should not include `helengine.AnimationPlayerComponent`
- `DemoDiscOverlayImage` should show `Scale=0.95,1,1`

## Task 4: Build and Runtime Verify on DS

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
- the top-screen logo is static
- only X scaling is fractional
