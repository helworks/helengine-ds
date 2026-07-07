# DS Size Reduction First-Pass Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reduce Nintendo DS executable size with the highest-yield first-pass changes, starting with generic generated-core feature stripping and then removing DS-specific runtime overhead that does not need to ship in the optimized ROM.

**Architecture:** Push the primary reduction seam into helengine's existing generated-core feature system instead of adding DS-only codegen behavior. Treat DS as the forcing function, but keep the implementation generic by allowing platform builders to force-disable generated-core features before native source is emitted. After the generic seam exists, tighten the DS native runtime by removing avoidable diagnostics and heavyweight container or RTTI usage in the render path. Verify progress with focused engine tests, DS source-audit tests, and before/after ROM size measurements.

**Tech Stack:** C#, xUnit, helengine editor/codegen pipeline, C++ Nintendo DS runtime, PowerShell build scripts, Dockerized devkitARM build

---

## File Map

- Modify: `C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\RuntimeGenerationContract.cs`
  Extends the generic runtime-generation contract so platform builders can request generated-core feature removals explicitly.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformPreprocessorSymbolService.cs`
  Keeps generated-core symbol resolution aligned with the new generic feature-disable seam.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
  Applies forced-disabled generated-core features while producing the generated native source tree and build metadata.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorRuntimeNativeManifestWriter.cs`
  Removes unnecessary runtime native outputs from DS-style builds once the generic feature seam is available.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\codegen\features\helengine-feature-catalog.json`
  Declares the generic feature ownership that allows host I/O and text-processing branches to be stripped safely.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
  Adds engine-level coverage for forced-disabled generated-core features and emitted build arguments.
- Modify: `builder.tests/NintendoDsGeneratedCoreStagerTests.cs`
  Audits the DS staged generated-core output so stripped runtime files and includes stay removed.
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
  Covers DS runtime boot and diagnostics source cleanup.
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Covers DS 2D renderer container and overlay cleanup.
- Modify: `builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs`
  Covers DS 3D renderer container and RTTI cleanup.
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
  Removes or gates nonessential diagnostics and startup formatting overhead.
- Modify: `src/platform/ds/NintendoDsRuntimeDiagnosticsProvider.cpp`
  Trims diagnostics text generation that is not needed in optimized DS builds.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Replaces heavyweight text-state storage choices if the audit shows large avoidable STL usage.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Removes avoidable overlay formatting or container overhead from the 2D renderer.
- Modify: `src/platform/ds/NintendoDsRenderManager3D.hpp`
  Declares lighter-weight scene state structures if the 3D render path no longer needs current container behavior.
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`
  Removes avoidable RTTI and heavyweight container usage from the 3D renderer.

## Baseline

- Optimized current DS ROM: `C:\dev\helprojs\city\ds-generic-feature-overrides\helengine_ds.nds`
- Current optimized ROM size: `2,254,848` bytes
- No-LTO attribution ROM size: `2,311,168` bytes
- Remaining LTO benefit still preserved by current pipeline: `56,320` bytes

Observed biggest structural pressure:

- `helengine_core_unity.o` remains the largest single native object by far
- staged generic generated-core still pulls `std::filesystem`, stream parsing, and string-processing code into DS builds
- DS 2D and 3D render managers still carry noticeable container, formatting, and RTTI overhead

### Task 1: Add Red Tests for the Generic Forced-Disabled Feature Seam

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\RuntimeGenerationContract.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Add contract coverage for forced-disabled generated-core features**

Extend `RuntimeGenerationContract` with one explicit collection for generic generated-core feature ids that the active platform wants disabled even when the project graph would otherwise allow them.

Keep this seam generic:

- do not introduce DS-specific feature names into codegen
- do not map math or native platform concepts in codegen
- keep the contract focused on generated-core feature selection only

- [ ] **Step 2: Add failing engine tests**

Add focused tests in `EditorGeneratedCoreRegenerationServiceTests` that assert:

- forced-disabled feature ids from `RuntimeGenerationContract` are emitted into generated-core build arguments
- the feature-resolution step removes those features from the final enabled feature set
- the result is stable when the project graph would otherwise enable `host_file_system`, `runtime_json`, or `text_processing`

- [ ] **Step 3: Run the focused engine tests and verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter EditorGeneratedCoreRegenerationServiceTests
```

Expected:

- FAIL
- failures should show the new contract member and forced-disabled feature behavior are not implemented yet

### Task 2: Implement Generic Generated-Core Feature Stripping

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\RuntimeGenerationContract.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformPreprocessorSymbolService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\codegen\features\helengine-feature-catalog.json`

- [ ] **Step 1: Extend the runtime-generation contract**

Add the new forced-disabled generated-core feature collection to `RuntimeGenerationContract` and preserve constructor validation.

Use the existing runtime contract as the one place where a platform builder declares generated runtime expectations.

- [ ] **Step 2: Feed the new feature disables into generated-core regeneration**

Update `EditorGeneratedCoreRegenerationService` so the generated-core build-argument model receives the forced-disabled feature ids and resolves the final feature set accordingly.

Important:

- keep the mechanism generic across all platforms
- preserve current behavior for platforms that do not provide forced-disabled features
- do not patch generated files after emission

- [ ] **Step 3: Adjust feature catalog ownership only where needed**

Update `helengine-feature-catalog.json` only if the current ownership is too coarse to strip DS-unneeded branches cleanly.

The first-pass target is to make it legal to remove:

- `host_file_system`
- `runtime_json`
- `text_processing`

without breaking the DS runtime path that already embeds startup, scene, and code-module manifests natively.

- [ ] **Step 4: Re-run engine tests**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter EditorGeneratedCoreRegenerationServiceTests
```

Expected:

- PASS

- [ ] **Step 5: Commit the generic seam**

```powershell
git -C C:\dev\helworks\helengine add engine\helengine.baseplatform\Definitions\RuntimeGenerationContract.cs engine\helengine.editor\managers\project\EditorPlatformPreprocessorSymbolService.cs engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs engine\helengine.editor\codegen\features\helengine-feature-catalog.json engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: allow platforms to disable generated-core features"
```

### Task 3: Strip Unneeded Runtime Native Outputs From DS Builds

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorRuntimeNativeManifestWriter.cs`
- Modify: `builder.tests/NintendoDsGeneratedCoreStagerTests.cs`

- [ ] **Step 1: Add a failing DS staged-output audit**

Add or extend `NintendoDsGeneratedCoreStagerTests` so DS staged generated-core output is audited for the first-pass removal goals:

- no host-file-system-backed startup manifest reader path
- no runtime JSON reader path when manifests are emitted as native arrays or constants
- no staged generated-core file that exists only to support stripped features

The audit should verify concrete file absence or concrete include/content absence, not vague size expectations.

- [ ] **Step 2: Remove DS-unneeded runtime native outputs or references**

Update `EditorRuntimeNativeManifestWriter` and any generation flow it depends on so DS builds do not emit or include native runtime outputs that are only required by stripped generated-core features.

Use the generic forced-disabled feature seam to decide what is safe to omit.

- [ ] **Step 3: Re-run the DS staged-output audit**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsGeneratedCoreStagerTests
```

Expected:

- PASS

### Task 4: Trim DS Boot and Diagnostics Overhead

**Files:**
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
- Modify: `src/platform/ds/NintendoDsRuntimeDiagnosticsProvider.cpp`

- [ ] **Step 1: Add failing DS boot-host audits**

Add source-audit tests that lock in the targeted cleanup:

- avoid startup string formatting or diagnostic paths that are not required in optimized DS builds
- avoid duplicated overlay text generation work in the boot/runtime diagnostics path

The audit should assert on concrete removed code paths or replacement helpers.

- [ ] **Step 2: Implement the DS diagnostics diet**

Simplify the DS boot/diagnostics sources so optimized builds do not pay for formatting-heavy or redundant diagnostics behavior.

Keep behavior rules clear:

- do not remove required fatal error reporting
- do not hide real boot failures
- do remove nice-to-have diagnostic text that costs code size and is not essential on DS

- [ ] **Step 3: Run the boot-host audit**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsBootHostSourceAuditTests
```

Expected:

- PASS

### Task 5: Remove DS Renderer Container and RTTI Overhead

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`

- [ ] **Step 1: Add failing DS renderer audits**

Add focused source-audit coverage for the largest DS-native cleanup targets:

- remove `std::unordered_map` from the 2D hardware-text submission path if a lighter structure is sufficient
- remove avoidable `dynamic_cast` usage in the 3D render path
- remove formatting-heavy performance overlay helpers from release-size-sensitive code paths when they are not needed

- [ ] **Step 2: Replace the heavyweight render-path structures**

Implement the smallest safe replacements in the DS render managers.

Examples of acceptable changes:

- linear state arrays or vectors where keyed hash lookup is unnecessary
- type-tag or interface-driven routing that avoids RTTI in hot render submission paths
- simpler overlay text assembly that does not pull in extra formatting machinery

Do not change scene authoring, renderer behavior, or lighting correctness as part of this task.

- [ ] **Step 3: Run the DS renderer audits**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager3DSourceAuditTests
```

Expected:

- PASS

### Task 6: Measure the First-Pass Win End-to-End

**Files:**
- Verify: `C:\dev\helprojs\city\ds-generic-feature-overrides\helengine_ds.nds`
- Verify: `C:\dev\helworks\helengine-ds\build-size-attrib\helengine_ds_size_attrib.map`

- [ ] **Step 1: Build the optimized DS ROM**

Run:

```powershell
rtk proxy powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform ds -Output C:\dev\helprojs\city\ds-generic-feature-overrides
```

Expected:

- Build completed for platform `ds`

- [ ] **Step 2: Record the new ROM size**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -Command "(Get-Item 'C:\dev\helprojs\city\ds-generic-feature-overrides\helengine_ds.nds').Length"
```

Expected:

- new size is lower than `2,254,848`

- [ ] **Step 3: If needed, produce one fresh no-LTO attribution build**

Only if the first-pass optimized win is smaller than expected, regenerate the no-LTO attribution build and compare:

- `helengine_core_unity.o`
- `NintendoDsRenderManager2D.o`
- `NintendoDsRenderManager3D.o`
- `NintendoDsBootHost.o`

Use the same attribution method as the current baseline so results stay comparable.

- [ ] **Step 4: Manual smoke-check the ROM**

Verify:

- ROM boots
- main menu renders
- one 2D-heavy scene renders
- one 3D-heavy scene renders
- bottom-screen UI still works

- [ ] **Step 5: Commit the DS first-pass reduction**

Commit in the owning repositories after verification, keeping generic engine changes separate from DS runtime changes.

## Stop Conditions

- If stripping `text_processing` also removes required shader or material behavior from DS runtime scenes, stop and split the feature catalog more precisely before proceeding.
- If removing runtime JSON or host-file-system support breaks the embedded manifest path, stop and fix the generic runtime ownership instead of adding DS-only generated-file rewrites.
- If RTTI removal from the DS renderers changes rendering behavior, keep the red audit, revert the behavior change, and switch that subtask to a smaller structural replacement.

## Expected Outcome

After this first pass:

- DS builds use a generic helengine-owned mechanism to request generated-core feature removal
- staged DS generated-core stops carrying avoidable host I/O and text-processing baggage
- DS native runtime sheds boot/overlay/render-path overhead that is not justified by the shipping ROM
- the optimized DS ROM drops meaningfully below `2,254,848` bytes without regressing rendering or menu behavior
