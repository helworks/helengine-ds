# SceneManager-Only Public Scene Transitions Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `SceneManager` the only public runtime scene-transition API, migrate direct `RuntimeSceneLoadService` callers to scene-id-based transitions, and keep `RuntimeSceneLoadService` only as an internal materialization helper.

**Architecture:** Extend `SceneManager` so it can resolve scene ids in both packaged-runtime and editor-authored contexts, then migrate public callers off `Core.SceneLoadService`. Keep `RuntimeSceneLoadService` alive underneath `SceneManager` for entity/component materialization and tracked asset resolution, but remove it from the public runtime contract.

**Tech Stack:** C#, xUnit, shared runtime core, editor test harness, generated-core normalization tests

---

## File Structure

### Shared runtime files

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
  - Hide the public `SceneLoadService` escape hatch from runtime callers and keep the service as an internal collaborator used to construct `SceneManager`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneManager.cs`
  - Add the scene-id-only public transition contract and editor-scene-id resolution path.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\MenuComponent.cs`
  - Route all scene transitions through `SceneManager`, including the editor-authored branch.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeSceneLoadService.cs`
  - Keep behavior intact, but make its role explicitly internal-facing in comments and call sites.

### Shared test files

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneManagerTests.cs`
  - Add scene-manager tests for editor-scene-id loading and remove dependence on `core.SceneLoadService` as a public test seam.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\RuntimeSceneLoadServiceTests.cs`
  - Keep direct materialization tests by constructing `RuntimeSceneLoadService` explicitly instead of reaching it through `Core`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.physics3d.tests\PhysicsWorld3DSceneLoadTests.cs`
  - Replace `core.SceneLoadService` test usage with explicit `RuntimeSceneLoadService` construction where raw materialization is still intentional.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\physics\PhysicsValidationSceneFactoryTests.cs`
  - Same change as physics runtime tests: direct loader instantiation instead of public `Core` access.

### Generated-core normalization tests

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
  - Update the menu/return-to-menu native normalization expectations so generated callers use `SceneManager` for scene transitions.

---

### Task 1: Lock the public API refactor with failing tests

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneManagerTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`

- [ ] **Step 1: Write the failing shared scene-manager tests**

```csharp
[Fact]
public void LoadScene_whenRuntimeSceneCatalogIsUnavailableButScenePathResolverCanResolveSceneId_loadsTheAuthoredSceneThroughSceneManager() {
    WriteSceneAsset("Scenes/AuthoredMenu.helen", 1u);
    Core core = CreateCore(
        sceneCatalog: null,
        scenePathResolver: new FakeScenePathResolver(("Scenes/AuthoredMenu.helen", "Scenes/AuthoredMenu.helen")));

    core.SceneManager.LoadScene("Scenes/AuthoredMenu.helen", SceneLoadMode.Single);

    LoadedSceneRecord loadedScene = Assert.Single(core.SceneManager.LoadedScenes);
    Assert.Equal("Scenes/AuthoredMenu.helen", loadedScene.SceneId);
}

[Fact]
public void LoadScene_whenNeitherSceneCatalogNorScenePathResolverCanResolveSceneId_throwsClearInvalidOperationException() {
    Core core = CreateCore(sceneCatalog: null, scenePathResolver: null);

    InvalidOperationException exception = Assert.Throws<InvalidOperationException>(
        () => core.SceneManager.LoadScene("Scenes/Missing.helen", SceneLoadMode.Single));

    Assert.Contains("scene id", exception.Message, StringComparison.OrdinalIgnoreCase);
}
```

- [ ] **Step 2: Write the failing generated-core normalization expectations**

```csharp
Assert.DoesNotContain("Core->Instance->SceneLoadService->Load(sceneAsset);", normalizedSource, StringComparison.Ordinal);
Assert.Contains("Core::get_Instance()->get_SceneManager()->LoadScene(PlatformMenuSceneResolver::ResolveMainMenuSceneId(), SceneLoadMode::Single);", normalizedSource);
Assert.DoesNotContain("if (Core.Instance.SceneLoadService == null)", normalizedManagedSource, StringComparison.Ordinal);
Assert.Contains("if (Core.Instance.SceneManager == null)", normalizedManagedSource);
```

- [ ] **Step 3: Run the narrow tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneManagerTests|FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests" --no-restore -v minimal
```

Expected:
- `SceneManagerTests` fails because `SceneManager` cannot yet resolve authored scene ids without a runtime catalog.
- generated-core normalization assertions fail because direct `SceneLoadService` strings are still present.

- [ ] **Step 4: Commit the red tests**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/SceneManagerTests.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorGeneratedCoreRegenerationServiceTests.cs
git commit -m "test: define SceneManager-only scene transition contract"
```

---

### Task 2: Teach SceneManager to own all public scene-id transitions

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneManager.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneManagerTests.cs`

- [ ] **Step 1: Add the failing implementation seam in `SceneManager`**

```csharp
readonly IScenePathResolver ScenePathResolver;

public SceneManager(
    RuntimeSceneCatalog sceneCatalog,
    ContentManager contentManager,
    RuntimeSceneLoadService sceneLoadService,
    ObjectManager objectManager,
    IScenePathResolver scenePathResolver) {
    SceneCatalog = sceneCatalog;
    ContentManager = contentManager ?? throw new ArgumentNullException(nameof(contentManager));
    SceneLoadService = sceneLoadService ?? throw new ArgumentNullException(nameof(sceneLoadService));
    ObjectManager = objectManager ?? throw new ArgumentNullException(nameof(objectManager));
    ScenePathResolver = scenePathResolver;
}
```

- [ ] **Step 2: Implement scene-id resolution in `SceneManager`**

```csharp
ResolvedSceneLoadTarget ResolveSceneLoadTarget(string sceneId) {
    if (SceneCatalog != null && SceneCatalog.TryGetEntry(sceneId, out RuntimeSceneCatalogEntry entry)) {
        return new ResolvedSceneLoadTarget(sceneId, entry.CookedRelativePath);
    }

    if (ScenePathResolver != null) {
        string authoredScenePath = ScenePathResolver.ResolveScenePath(sceneId);
        if (string.IsNullOrWhiteSpace(authoredScenePath)) {
            throw new InvalidOperationException($"Runtime scene id '{sceneId}' did not resolve to an authored scene path.");
        }

        return new ResolvedSceneLoadTarget(sceneId, authoredScenePath);
    }

    throw new InvalidOperationException($"Runtime scene id '{sceneId}' could not be resolved because no runtime scene catalog or scene path resolver is available.");
}
```

- [ ] **Step 3: Route `LoadSceneImmediate` through the resolved target**

```csharp
ResolvedSceneLoadTarget target = ResolveSceneLoadTarget(sceneId);
SceneLoading?.Invoke(this, new SceneLoadingEventArgs(target.SceneId, target.ContentPath));
SceneAsset sceneAsset = ContentManager.Load<SceneAsset>(target.ContentPath, RuntimeContentProcessorIds.SceneAsset);
RuntimeSceneLoadResult loadResult = SceneLoadService.LoadTracked(sceneAsset);
LoadedSceneRecord loadedSceneRecord = new LoadedSceneRecord(target.SceneId, target.ContentPath, loadResult.RootEntities, loadResult.OwnedAssets);
```

- [ ] **Step 4: Update `Core` so callers no longer need `Core.SceneLoadService`**

```csharp
internal RuntimeSceneLoadService SceneLoadService { get; private set; }

SceneManager CreateSceneManager(ContentManager contentManager, RuntimeSceneCatalog sceneCatalog) {
    if (contentManager == null) {
        throw new ArgumentNullException(nameof(contentManager));
    }
    if (sceneCatalog == null && InitializationOptions.ScenePathResolver == null) {
        return null;
    }

    return new SceneManager(
        sceneCatalog,
        contentManager,
        SceneLoadService,
        ObjectManager,
        InitializationOptions.ScenePathResolver);
}
```

- [ ] **Step 5: Run the narrow tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneManagerTests.Initialize_whenRuntimeSceneCatalogIsProvided_createsSceneManager|FullyQualifiedName~SceneManagerTests.LoadScene_whenRuntimeSceneCatalogIsUnavailableButScenePathResolverCanResolveSceneId_loadsTheAuthoredSceneThroughSceneManager|FullyQualifiedName~SceneManagerTests.LoadScene_whenNeitherSceneCatalogNorScenePathResolverCanResolveSceneId_throwsClearInvalidOperationException" --no-restore -v minimal
```

Expected:
- `3 tests passed`

- [ ] **Step 6: Commit the SceneManager refactor**

```bash
git add C:/dev/helworks/helengine/engine/helengine.core/Core.cs C:/dev/helworks/helengine/engine/helengine.core/scene/runtime/SceneManager.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/SceneManagerTests.cs
git commit -m "feat: route public scene transitions through SceneManager"
```

---

### Task 3: Migrate shared callers off public `SceneLoadService`

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\MenuComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\RuntimeSceneLoadServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`

- [ ] **Step 1: Replace `MenuComponent` editor and runtime branches with one SceneManager path**

```csharp
void LoadScene(string sceneId) {
    if (string.IsNullOrWhiteSpace(sceneId)) {
        throw new InvalidOperationException("Scene-loading baked menu items must provide a scene id.");
    }
    if (Core.Instance == null) {
        throw new InvalidOperationException("A core instance must exist before loading a scene from the baked menu.");
    }
    if (Core.Instance.SceneManager == null) {
        throw new InvalidOperationException("Core scene manager must be initialized before menu scene loading can occur.");
    }

    Core.Instance.SceneManager.LoadScene(sceneId, SceneLoadMode.Single);
    if (ComponentExecutionContext.CurrentMode == ComponentExecutionMode.Editor && Parent != null) {
        Parent.Enabled = false;
    }
}
```

- [ ] **Step 2: Update generated-core normalization expectations**

```csharp
Assert.Contains("Core::get_Instance()->get_SceneManager()->LoadScene(PlatformMenuSceneResolver::ResolveMainMenuSceneId(), SceneLoadMode::Single);", normalizedSource);
Assert.DoesNotContain("Core::get_Instance()->get_SceneLoadService()->Load(sceneAsset);", normalizedSource, StringComparison.Ordinal);
Assert.DoesNotContain("Core.Instance.SceneLoadService", normalizedManagedSource, StringComparison.Ordinal);
Assert.Contains("Core.Instance.SceneManager.LoadScene(sceneId, SceneLoadMode.Single);", normalizedManagedSource);
```

- [ ] **Step 3: Update raw materialization tests to construct the loader explicitly**

```csharp
RuntimeSceneAssetReferenceResolver resolver = new RuntimeSceneAssetReferenceResolver(
    Core.Instance.ContentManager,
    TempRootPath,
    ShaderCompileTarget.DirectX11);
RuntimeSceneLoadService loadService = new RuntimeSceneLoadService(resolver, RuntimeComponentRegistry.CreateDefault());
IReadOnlyList<Entity> startupRoots = loadService.Load(startupSceneAsset);
```

- [ ] **Step 4: Run the focused shared tests**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneLoadServiceTests|FullyQualifiedName~SceneManagerTests|FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests.Normalize_generated_native_sources_fixes_demo_disc_return_to_menu_component_windows_translation" --no-restore -v minimal
```

Expected:
- menu and scene-manager tests pass
- generated-core normalization tests no longer expect direct `SceneLoadService` scene transitions

- [ ] **Step 5: Commit the caller migration**

```bash
git add C:/dev/helworks/helengine/engine/helengine.core/components/2d/menu/MenuComponent.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorGeneratedCoreRegenerationServiceTests.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/RuntimeSceneLoadServiceTests.cs
git commit -m "refactor: migrate menu scene transitions to SceneManager"
```

---

### Task 4: Remove remaining test and helper dependence on `Core.SceneLoadService`

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.physics3d.tests\PhysicsWorld3DSceneLoadTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\physics\PhysicsValidationSceneFactoryTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneManagerTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.physics3d.tests\helengine.physics3d.tests.csproj`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`

- [ ] **Step 1: Replace remaining raw-loader test usage with explicit service construction**

```csharp
RuntimeSceneAssetReferenceResolver resolver = new RuntimeSceneAssetReferenceResolver(
    core.ContentManager,
    contentRootPath,
    ShaderCompileTarget.DirectX11);
RuntimeSceneLoadService loadService = new RuntimeSceneLoadService(resolver, RuntimeComponentRegistry.CreateDefault());
IReadOnlyList<Entity> rootEntities = loadService.Load(sceneAsset);
```

- [ ] **Step 2: Keep the untracked-startup-roots regression explicit in `SceneManagerTests`**

```csharp
RuntimeSceneAssetReferenceResolver resolver = new RuntimeSceneAssetReferenceResolver(
    core.ContentManager,
    TempRootPath,
    ShaderCompileTarget.DirectX11);
RuntimeSceneLoadService startupLoadService = new RuntimeSceneLoadService(resolver, RuntimeComponentRegistry.CreateDefault());
IReadOnlyList<Entity> startupRoots = startupLoadService.Load(startupSceneAsset);
```

- [ ] **Step 3: Run the affected test projects**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.physics3d.tests\helengine.physics3d.tests.csproj --no-restore -v minimal
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneManagerTests|FullyQualifiedName~PhysicsValidationSceneFactoryTests" --no-restore -v minimal
```

Expected:
- physics scene-load tests pass without using `core.SceneLoadService`
- scene-manager regression tests still pass with explicit loader setup where intentional

- [ ] **Step 4: Run the direct-usage inventory check**

Run:

```powershell
rg -n "Core\\.Instance\\.SceneLoadService|core\\.SceneLoadService" C:\dev\helworks\helengine\engine -g "*.cs"
```

Expected:
- no remaining public/runtime call sites in shared engine code
- any remaining results should be editor-only `SceneLoadService` types or explicit local `RuntimeSceneLoadService` construction in tests

- [ ] **Step 5: Commit the cleanup**

```bash
git add C:/dev/helworks/helengine/engine/helengine.physics3d.tests/PhysicsWorld3DSceneLoadTests.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/physics/PhysicsValidationSceneFactoryTests.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/SceneManagerTests.cs
git commit -m "test: remove public SceneLoadService usage from shared tests"
```

---

### Task 5: Final verification and DS regression check

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\docs\superpowers\plans\2026-05-17-scene-manager-only-public-scene-transitions.md`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`
- Test: `C:\dev\helworks\helengine\engine\helengine.physics3d.tests\helengine.physics3d.tests.csproj`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj`
- Manual: `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

- [ ] **Step 1: Run the full shared verification slice**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneManagerTests|FullyQualifiedName~RuntimeSceneLoadServiceTests|FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests" --no-restore -v minimal
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.physics3d.tests\helengine.physics3d.tests.csproj --no-restore -v minimal
```

Expected:
- all targeted shared tests pass

- [ ] **Step 2: Run the DS source-audit slice that guards startup/scene transitions**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore -v minimal
```

Expected:
- DS startup/scene-transition audits pass

- [ ] **Step 3: Rebuild the DS ROM from the current engine mainline**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected:
- `Build completed for platform 'ds': C:\tmp\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 4: Verify the runtime symptom manually in `melonDS`**

Manual check:

```text
1. Boot into the DS menu.
2. Record `Mem used` and `Own tex/font/mat/mdl`.
3. Enter `cube_test`.
4. Press `B` to return.
5. Record `Mem used` and `Own tex/font/mat/mdl` again.
```

Expected:
- startup menu and returned menu report the same tracked owned-asset classes
- scene-return memory no longer jumps because the startup menu came through a bypass path

- [ ] **Step 5: Commit final integration adjustments**

```bash
git add C:/dev/helworks/helengine C:/dev/helworks/helengine-ds
git commit -m "refactor: make SceneManager the public scene transition API"
```
