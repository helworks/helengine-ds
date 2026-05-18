# CsharpCodegen C++ Ownership And Engine Rewrite Removal Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move every native C++ ownership/lifetime fix out of `helengine` post-generation rewrites and into `csharpcodegen`, then remove the engine-side rewrite layer entirely so generated C++ is either correct at emission time or the build fails.

**Architecture:** Treat C++ lifetime as a generator-owned contract. `csharpcodegen` must emit the required ownership helpers, destruction paths, and scope-exit guards directly from source/runtime metadata and templates. `helengine` must stop mutating generated C++ after conversion; at most it may consume a generator success/failure contract, but it must never rewrite output.

**Tech Stack:** `csharpcodegen` (`cs2.cpp` emitter, runtime templates, compile-validation tests), `helengine` editor build pipeline, generated C++ runtime support.

---

## Scope

- Remove **all** native C++ post-generation rewrites from:
  - `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
- Replace them with generator-owned emission/template/runtime fixes in:
  - `C:\dev\helworks\csharpcodegen\cs2.cpp\...`
  - `C:\dev\helworks\csharpcodegen\cs2.cpp\.net.cpp\...`
- Add generator-owned validation so bad ownership output fails the build instead of shipping and relying on downstream repairs.

## Non-Goals

- Do not keep a mirrored regex/post-pass in `csharpcodegen`.
- Do not leave “temporary compatibility” rewrites in `helengine`.
- Do not paper over generator defects with best-effort mutations.

## Current Rewrite Inventory To Eliminate

Engine-side rewrites currently exist for:

- `ContentManager.cpp` temporary stream ownership
- `RenderManager2D.cpp` font ownership
- `RuntimeSceneAssetReferenceResolver.cpp` transient asset ownership and tracking-container ownership
- `SceneManager.cpp` scene asset / load result / loaded-scene-record ownership
- `Entity.cpp` recursive entity/component/container disposal
- `ObjectManager.cpp` pending update operation ownership
- `FontAsset.cpp` source texture ownership
- `FontAssetBinarySerializer.cpp` version constant patching
- `float4` out-value temporary rewrite
- Several generic runtime helper insertions (`native_dictionary`, `array`, `native_string`, feature manifest, etc.)

These must all end up in one of two places:

1. **Emitter logic** in `csharpcodegen`
2. **Runtime template source** under `cs2.cpp/.net.cpp`

If a case cannot fit either category, the design is incomplete and implementation should stop.

---

### Task 1: Freeze The Rewrite Surface And Capture The Required Contracts

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Add characterization tests that enumerate every existing native rewrite entry point**

Write/extend tests so they assert the exact current rewrite inventory. The purpose is to lock the removal scope, not preserve the behavior forever.

- [ ] **Step 2: Add one red test that describes the desired end state**

The new end-state test should assert that `NormalizeGeneratedNativeSources(...)` performs no native file rewrites and either:
- is deleted entirely, or
- throws a clear exception if called for native output normalization

Expected failure initially: current engine code still rewrites multiple native files.

- [ ] **Step 3: Do not change production code yet**

This task is only about locking scope. It should produce a failing end-state test and a clear inventory of all rewrites that must be migrated.

- [ ] **Step 4: Commit**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorGeneratedCoreRegenerationService.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorGeneratedCoreRegenerationServiceTests.cs
git commit -m "test: lock native rewrite removal scope"
```

---

### Task 2: Classify Each Rewrite As Emitter Logic Or Runtime Template Source

**Files:**
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOutputNormalizer.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPConversiorProcessor.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPClassEmitter.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPCompileValidationRegressionTests.cs`
- Test: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPCompileValidationRegressionTests.cs`

- [ ] **Step 1: Create a migration matrix in test comments or test names**

For each engine rewrite, decide whether the fix belongs in:
- `CPPConversiorProcessor`
- `CPPClassEmitter`
- `CPPGeneratedOutputNormalizer`
- `.net.cpp` runtime template files

The expected mapping is:
- generic helper/header/runtime defects -> template source
- language lowering / scope lifetime / temporary destruction -> emitter logic
- no regex/text replacement allowed for ownership-sensitive runtime code

- [ ] **Step 2: Add red compile-validation tests in `csharpcodegen` for each migrated contract**

At minimum cover:
- scene-load temporary asset cleanup
- resolver generated/cooked asset cleanup
- entity/component/tree disposal
- pending operation deletion
- font/source texture cleanup
- serializer version constant correctness

Expected failure initially: `csharpcodegen` does not emit all of these directly.

- [ ] **Step 3: Keep `CPPGeneratedOutputNormalizer` only for generic template-safe cases**

The plan target is:
- ownership-sensitive repairs must leave this class
- if a normalization still exists after migration, it must be generic, backend-owned, and non-engine-specific

- [ ] **Step 4: Commit**

```bash
git add C:/dev/helworks/csharpcodegen/cs2.cpp C:/dev/helworks/csharpcodegen/cs2.cpp.tests
git commit -m "test: define codegen-owned native ownership contracts"
```

---

### Task 3: Move Runtime Support Fixes Into `.net.cpp` Templates

**Files:**
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\.net.cpp\**\*.hpp`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\.net.cpp\**\*.cpp`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOutputNormalizer.cs`
- Test: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPCompileValidationRegressionTests.cs`

- [ ] **Step 1: Move helper insertions into the actual runtime template files**

This includes items such as:
- `native_dictionary.hpp` `Clear()`
- `array.hpp` initialization behavior if still required
- `native_string.hpp` helper methods
- `number.hpp` finite helpers
- feature manifest/runtime helper gaps

These must be authored in the template source, not inserted later.

- [ ] **Step 2: Remove the corresponding normalizer entries**

After each template is fixed, delete the matching `CPPGeneratedOutputNormalizer` mutation.

- [ ] **Step 3: Run compile-validation tests**

Run:

```bash
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --filter "FullyQualifiedName~CPPCompileValidationRegressionTests" --no-restore -v minimal
```

Expected: the migrated helper/runtime tests pass without relying on post-generation insertion.

- [ ] **Step 4: Commit**

```bash
git add C:/dev/helworks/csharpcodegen/cs2.cpp/.net.cpp C:/dev/helworks/csharpcodegen/cs2.cpp/CPPGeneratedOutputNormalizer.cs C:/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPCompileValidationRegressionTests.cs
git commit -m "refactor: move generic runtime fixes into cpp templates"
```

---

### Task 4: Move Ownership Emission Into The C++ Emitter

**Files:**
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPConversiorProcessor.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPClassEmitter.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOutputAdapter.cs`
- Test: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPCompileValidationRegressionTests.cs`

- [ ] **Step 1: Define where ownership is emitted**

Ownership must be emitted from structured conversion state, not patched into final text. The implementation should add explicit generation rules for:
- temporary loaded asset scope guards
- queue-node deletion after flush/apply
- transferred ownership vs copied ownership of lists/arrays
- entity/component recursive destruction where the converted C# semantics require it

- [ ] **Step 2: Implement emitter-side generation for transient asset scope guards**

Target contracts:
- `SceneManager::LoadSceneImmediate(...)`
- `RuntimeSceneAssetReferenceResolver::{ResolveModel, ResolveMaterial, ResolveTexture, ResolveFont, ApplyMaterialDiffuseTexture}`
- other generated functions currently depending on engine rewrites

The emitter should understand these as structured output, not regex replacements.

- [ ] **Step 3: Implement emitter-side generation for destruction/queue ownership**

Target contracts:
- `Entity::Dispose()`
- `ObjectManager::ApplyPendingUpdateOperations()`
- `SceneManager::UnloadSceneImmediate(...)`
- any generated `RuntimeSceneOwnedAssetSet` container handoff points

- [ ] **Step 4: Add compile-validation tests that assert the exact emitted code shape**

Use the existing compile-validation suite to check for:
- `he_cpp_make_scope_exit` where required
- `delete operation;`
- correct transfer vs clone behavior
- no missing ownership release in generated cooked/material/model flows

- [ ] **Step 5: Commit**

```bash
git add C:/dev/helworks/csharpcodegen/cs2.cpp/CPPConversiorProcessor.cs C:/dev/helworks/csharpcodegen/cs2.cpp/CPPClassEmitter.cs C:/dev/helworks/csharpcodegen/cs2.cpp/CPPGeneratedOutputAdapter.cs C:/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPCompileValidationRegressionTests.cs
git commit -m "feat: emit native ownership contracts in cpp generator"
```

---

### Task 5: Add A Fail-Loud Validation Gate In `csharpcodegen`

**Files:**
- Create: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPGeneratedOwnershipValidator.cs`
- Modify: `C:\dev\helworks\csharpcodegen\cs2.cpp\CPPCodeConverter.cs`
- Test: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPCompileValidationRegressionTests.cs`

- [ ] **Step 1: Add a generated-output validator that runs after emission and before handoff**

This validator must not rewrite anything. It should only:
- inspect generated output
- detect missing ownership contracts that must have been emitted structurally
- throw a hard `InvalidOperationException` with the file path and missing contract

- [ ] **Step 2: Wire the validator into the codegen output pipeline**

Run it after:
- runtime templates are copied
- classes are emitted
- backend adapters run

But before:
- the output is considered final
- any downstream engine build consumes it

- [ ] **Step 3: Add tests for fail-loud behavior**

At least one test should intentionally feed malformed generated output and assert:
- no rewrite occurs
- conversion fails loudly with a message naming the missing contract

- [ ] **Step 4: Commit**

```bash
git add C:/dev/helworks/csharpcodegen/cs2.cpp/CPPGeneratedOwnershipValidator.cs C:/dev/helworks/csharpcodegen/cs2.cpp/CPPCodeConverter.cs C:/dev/helworks/csharpcodegen/cs2.cpp.tests/CPPCompileValidationRegressionTests.cs
git commit -m "feat: fail cpp generation on missing ownership contracts"
```

---

### Task 6: Delete Engine-Side Native Rewrites

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Remove every native rewrite branch from `NormalizeGeneratedNativeSources(...)`**

This includes deleting:
- `RewriteGeneratedContentManagerTemporaryStreamOwnership`
- `RewriteGeneratedRenderManager2DFontOwnership`
- `RewriteGeneratedRuntimeSceneAssetReferenceResolverTemporaryAssetOwnership`
- `RewriteGeneratedSceneManagerTemporarySceneAssetOwnership`
- `RewriteGeneratedEntityDisposeOwnership`
- `RewriteGeneratedObjectManagerPendingUpdateOperationOwnership`
- `RewriteGeneratedFontAssetSourceTextureOwnership`
- `RewriteGeneratedFontAssetBinarySerializerVersionConstants`
- any related helper regex/replace methods

- [ ] **Step 2: Preserve only non-native responsibilities if truly still needed**

If the method still exists after cleanup, it must not mutate generated native source files.

- [ ] **Step 3: Update tests to assert absence, not presence**

The editor-side tests should now verify:
- no native rewrite helpers remain
- the engine build path does not attempt to normalize generated C++ ownership/lifetime

- [ ] **Step 4: Run focused editor tests**

Run:

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests" --no-restore -v minimal
```

Expected: the test slice passes with zero reliance on native rewrite helpers.

- [ ] **Step 5: Commit**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorGeneratedCoreRegenerationService.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorGeneratedCoreRegenerationServiceTests.cs
git commit -m "refactor: remove engine-side native cpp rewrites"
```

---

### Task 7: End-To-End Validation Across Repos

**Files:**
- Modify: `C:\dev\helworks\helengine\user_settings\platforms.json` only if needed to point at the intended `csharpcodegen` / builder outputs
- Test: `C:\dev\helworks\csharpcodegen\cs2.cpp.tests\CPPCompileValidationRegressionTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\EditorGeneratedCoreRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\*.cs`

- [ ] **Step 1: Run focused `csharpcodegen` validation**

```bash
rtk dotnet test C:\dev\helworks\csharpcodegen\cs2.cpp.tests\cs2.cpp.tests.csproj --no-restore -v minimal
```

- [ ] **Step 2: Run focused `helengine` editor validation**

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests|FullyQualifiedName~RuntimeSceneAssetReferenceResolverSourceTests" --no-restore -v minimal
```

- [ ] **Step 3: Run focused DS builder validation**

```bash
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore -v minimal
```

- [ ] **Step 4: Run a fresh DS build**

```bash
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected:
- build succeeds
- no engine-side native rewrite code runs
- generated C++ output already contains correct ownership code

- [ ] **Step 5: Manual runtime validation**

In `melonDS`:
- record `Mem used` on menu boot
- enter `cube_test`
- press `B` back to menu
- confirm `Mem used` returns to the same baseline or within known allocator fragmentation noise

- [ ] **Step 6: Commit**

```bash
git add C:/dev/helworks/csharpcodegen C:/dev/helworks/helengine C:/dev/helworks/helengine-ds
git commit -m "fix: move cpp ownership generation into csharpcodegen"
```

---

## Self-Review

- Spec coverage: this plan covers generator ownership emission, template migration, fail-loud validation, engine rewrite deletion, and DS end-to-end verification.
- Placeholder scan: no TBD/TODO placeholders remain.
- Consistency: the plan consistently treats ownership fixes as `csharpcodegen` responsibilities and engine rewrites as forbidden end state.

## Expected Final State

- `helengine` no longer mutates generated native C++.
- `csharpcodegen` emits correct lifetime/ownership code directly.
- malformed ownership output fails during generation instead of being silently repaired.
- DS/GameCube/native scene roundtrips stop depending on hidden post-generation rewrites.
