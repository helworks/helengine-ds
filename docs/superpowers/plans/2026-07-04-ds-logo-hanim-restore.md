# DS Logo `.hanim` Restore Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore the Nintendo DS main-menu logo animation by attaching the existing `DemoDiscLogoIdle.hanim` clip to the DS logo entity.

**Architecture:** Keep the change localized to the city menu scene factory. Add one focused source-audit test that proves the DS logo path authors an `AnimationPlayerComponent` with autoplay, loop, live clip loading, and serialized clip references. Then regenerate the menu scene and verify through a DS build.

**Tech Stack:** C#, xUnit source-audit tests, city generated scene pipeline, PowerShell build scripts, melonDS

---

## File Map

- Create: `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs`
  Verifies the DS-specific menu-logo authoring path uses `AnimationPlayerComponent` with the existing `.hanim` clip contract.
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
  Reattaches the shared animation clip to the DS logo entity and serializes the clip reference.
- Create: `docs/superpowers/plans/2026-07-04-ds-logo-hanim-restore.md`
  Records the implementation steps for this behavior change.

## Task 1: Add the Failing Source Audit

**Files:**
- Create: `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing source-audit test**

Create `builder.tests/NintendoDsDemoDiscMenuSourceAuditTests.cs` with one test that reads `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs` and asserts the `CreateNintendoDsTopScreenLogoEntity(...)` body contains:

- `new AnimationPlayerComponent`
- `PlayAutomatically = true`
- `ShouldLoop = true`
- `LoadRequiredAnimationClipAsset(DemoDiscLogoIdleAnimationRelativePath)`
- `ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath)`

- [ ] **Step 2: Run the test to verify it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsDemoDiscMenuSourceAuditTests"
```

Expected:

- FAIL
- failure should show the DS logo method does not yet author the animation player

## Task 2: Reattach the `.hanim` Clip in the DS Menu Factory

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add the animation player to the DS logo entity**

In `CreateNintendoDsTopScreenLogoEntity(...)`, create one `AnimationPlayerComponent` configured with:

- `Clip = LoadRequiredAnimationClipAsset(DemoDiscLogoIdleAnimationRelativePath)`
- `PlayAutomatically = true`
- `ShouldLoop = true`

Then add it to the entity and serialize the clip reference with:

```csharp
ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath);
```

- [ ] **Step 2: Run the source-audit test to verify it passes**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsDemoDiscMenuSourceAuditTests"
```

Expected:

- PASS

## Task 3: Regenerate and Verify the DS Menu

**Files:**
- Modify: generated scene outputs under `C:\dev\helprojs\city\assets\scenes\`
- Verify: `C:\dev\helprojs\city\assets\scenes\DemoDiscMainMenuDs.helen`

- [ ] **Step 1: Regenerate the menu scene**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.regenerate-demo-disc-main-menu
```

Expected:

- `Editor command 'menu.regenerate-demo-disc-main-menu' executed successfully.`

- [ ] **Step 2: Optionally inspect the baked scene**

Run:

```powershell
rtk dotnet run --project .\scratch\SceneAssetDump\SceneAssetDump.csproj -- C:\dev\helprojs\city\assets\scenes\DemoDiscMainMenuDs.helen
```

Expected:

- `DemoDiscOverlayImage` should include `helengine.AnimationPlayerComponent`

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
- top-screen logo animates again
- no old quadrant/upright mismatch during rotation
