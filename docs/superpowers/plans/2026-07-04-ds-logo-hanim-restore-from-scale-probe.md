# DS Logo `.hanim` Restore From Scale Probe Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the DS static scale probe and restore the Nintendo DS menu logo to the shared `DemoDiscLogoIdle.hanim` animation path.

**Architecture:** Keep the change localized to the DS menu authoring path in the city project and one focused source-audit test in this repo. Restore the animation-player contract first in the test, then reattach the `AnimationPlayerComponent`, regenerate the baked menu scene, rebuild the DS ROM, and relaunch melonDS.

**Tech Stack:** C#, xUnit source-audit tests, city generated-scene pipeline, PowerShell build scripts, melonDS

---

## File Map

- Modify: `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs`
  Restores the `.hanim`-based expectations for the DS logo method.
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
  Removes the static probe scale and reattaches the DS `AnimationPlayerComponent`.
- Create: `docs/superpowers/plans/2026-07-04-ds-logo-hanim-restore-from-scale-probe.md`
  Records the implementation steps for the restore.

## Task 1: Write the Failing Source Audit

**Files:**
- Modify: `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Restore the DS logo source-audit expectations**

Update the existing DS logo source audit so it asserts the DS logo method:

- contains `AnimationPlayerComponent animationPlayerComponent = new AnimationPlayerComponent`
- contains `Clip = LoadRequiredAnimationClipAsset(DemoDiscLogoIdleAnimationRelativePath)`
- contains `PlayAutomatically = true`
- contains `ShouldLoop = true`
- contains `entity.AddComponent(animationPlayerComponent);`
- contains `ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath);`
- contains `entity.LocalScale = new float3(1f, 1f, 1f);`

- [ ] **Step 2: Run the test to verify it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsDemoDiscMenuSourceAuditTests"
```

Expected:

- FAIL
- failure should show the static-scale probe is still authored and the animation player is absent

## Task 2: Restore the `.hanim` Path

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Remove the static scale probe**

Set:

```csharp
entity.LocalScale = new float3(1f, 1f, 1f);
```

- [ ] **Step 2: Reattach the DS animation player**

Add one `AnimationPlayerComponent` configured with:

- `Clip = LoadRequiredAnimationClipAsset(DemoDiscLogoIdleAnimationRelativePath)`
- `PlayAutomatically = true`
- `ShouldLoop = true`

Then add it to the entity and serialize the clip reference with:

```csharp
ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath);
```

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

- `DemoDiscOverlayImage` should include `helengine.AnimationPlayerComponent`
- `DemoDiscOverlayImage` should show `Scale=1,1,1`

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
- the top-screen logo animates again
