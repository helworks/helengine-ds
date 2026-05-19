# SceneMapComponent Singleton Menu Resolution Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove `SceneMapService`, make `SceneMapComponent` a strict optional singleton helper, and route all menu scene loads through singleton-based scene-id resolution.

**Architecture:** Keep scene remapping lightweight and optional by moving it out of `Core` service ownership. `SceneMapComponent` becomes the runtime singleton and exposes one static resolver helper, while shared menu code and city menu code call that helper explicitly before `SceneManager.LoadScene(...)`.

**Tech Stack:** C#, helengine runtime/editor tests, city gameplay source project, DS editor-driven build verification

---

## File Structure

### Shared engine files

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\SceneMapComponent.cs`
  - add singleton lifecycle and static resolve helper
- Delete: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneMapService.cs`
  - remove the old `Core`-owned helper service
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
  - remove `SceneMapService` creation/storage
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\MenuComponent.cs`
  - resolve all menu scene loads through `SceneMapComponent`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\DemoDiscReturnToMenuRuntimeComponent.cs`
  - resolve the main-menu scene id through `SceneMapComponent`

### Shared engine tests

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
  - replace service-based tests with singleton component tests
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapComponentPersistenceDescriptorTests.cs`
  - keep persistence checks valid after the runtime-access change

### Project-owned gameplay files

- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`
  - use the singleton helper instead of hardcoded direct scene loads

### Verification targets

- Verify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`
- Verify: `C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj`
- Verify: `C:\dev\helprojs\city\project.heproj`

---

### Task 1: Replace SceneMapService Tests With Singleton Contract Tests

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Rewrite the test file around singleton-based behavior**

```csharp
namespace helengine.tests.serialization.scene {
    /// <summary>
    /// Verifies runtime singleton scene-map resolution behavior.
    /// </summary>
    public sealed class SceneMapServiceTests {
        /// <summary>
        /// Clears singleton state after each test so tests remain isolated.
        /// </summary>
        [TearDown]
        public void TearDown() {
            SceneMapComponent.Instance = null;
        }

        /// <summary>
        /// Verifies that missing scene-map state leaves the requested scene id unchanged.
        /// </summary>
        [Test]
        public void ResolveSceneId_WhenNoInstanceExists_ReturnsOriginalSceneId() {
            string resolvedSceneId = SceneMapComponent.ResolveSceneId("DemoDiscMainMenu");

            Assert.Equal("DemoDiscMainMenu", resolvedSceneId);
        }

        /// <summary>
        /// Verifies that one singleton scene-map component remaps the requested scene id.
        /// </summary>
        [Test]
        public void ResolveSceneId_WhenMappingExists_ReturnsMappedSceneId() {
            SceneMapComponent component = new SceneMapComponent();
            component.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");
            component.OnAttachedToEntityForTests();

            string resolvedSceneId = SceneMapComponent.ResolveSceneId("DemoDiscMainMenu");

            Assert.Equal("DemoDiscMainMenuDs", resolvedSceneId);
        }

        /// <summary>
        /// Verifies that enabling a second scene-map component fails immediately.
        /// </summary>
        [Test]
        public void Activate_WhenSecondInstanceAppears_ThrowsInvalidOperationException() {
            SceneMapComponent first = new SceneMapComponent();
            first.OnAttachedToEntityForTests();
            SceneMapComponent second = new SceneMapComponent();

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => second.OnAttachedToEntityForTests());

            Assert.Contains("SceneMapComponent", exception.Message, StringComparison.Ordinal);
        }

        /// <summary>
        /// Verifies that clearing the active singleton restores pass-through resolution.
        /// </summary>
        [Test]
        public void Detach_WhenInstanceIsCleared_ResolveSceneIdReturnsOriginalSceneId() {
            SceneMapComponent component = new SceneMapComponent();
            component.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");
            component.OnAttachedToEntityForTests();
            component.OnDetachedFromEntityForTests();

            string resolvedSceneId = SceneMapComponent.ResolveSceneId("DemoDiscMainMenu");

            Assert.Equal("DemoDiscMainMenu", resolvedSceneId);
        }
    }
}
```

- [ ] **Step 2: Run the singleton scene-map tests to verify they fail**

Run: `dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" --no-restore -v minimal`

Expected: FAIL because `SceneMapComponent.Instance`, `ResolveSceneId(...)`, and the singleton lifecycle hooks do not exist yet.

- [ ] **Step 3: Commit the failing test rewrite**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/serialization/scene/SceneMapServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "test: define scene map singleton runtime contract"
```

### Task 2: Implement SceneMapComponent Singleton And Remove SceneMapService

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\SceneMapComponent.cs`
- Delete: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneMapService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Implement the singleton and resolve helper on SceneMapComponent**

```csharp
namespace helengine {
    /// <summary>
    /// Stores optional scene-id remapping entries and exposes one strict runtime singleton helper.
    /// </summary>
    public sealed class SceneMapComponent : Component {
        /// <summary>
        /// Current cooked payload version used by runtime scene persistence.
        /// </summary>
        public const byte CurrentVersion = 1;

        /// <summary>
        /// Stable serialized component type id used by scene persistence.
        /// </summary>
        public const string SerializedComponentTypeId = "helengine.SceneMapComponent";

        /// <summary>
        /// Gets the currently active singleton scene-map component, when one exists.
        /// </summary>
        public static SceneMapComponent Instance { get; internal set; }

        /// <summary>
        /// Initializes one empty scene-map component.
        /// </summary>
        public SceneMapComponent() {
            Mappings = new Dictionary<string, string>(StringComparer.Ordinal);
        }

        /// <summary>
        /// Gets the authored mapping entries keyed by logical source scene id.
        /// </summary>
        [EditorPropertyDisplayName("Scene Mappings")]
        [EditorPropertyOrder(0)]
        public Dictionary<string, string> Mappings { get; }

        /// <summary>
        /// Resolves one logical scene id through the active singleton mapping when present.
        /// </summary>
        /// <param name="sceneId">Logical scene id to resolve.</param>
        /// <returns>Mapped scene id when one mapping exists; otherwise the original scene id.</returns>
        public static string ResolveSceneId(string sceneId) {
            if (string.IsNullOrWhiteSpace(sceneId)) {
                throw new ArgumentException("Scene id is required.", nameof(sceneId));
            }

            SceneMapComponent instance = Instance;
            if (instance == null) {
                return sceneId;
            }

            if (instance.Mappings.TryGetValue(sceneId, out string mappedSceneId)) {
                return mappedSceneId;
            }

            return sceneId;
        }

        /// <summary>
        /// Registers the current component as the singleton scene-map instance.
        /// </summary>
        protected override void OnEnabled() {
            base.OnEnabled();
            RegisterSingletonInstance();
        }

        /// <summary>
        /// Clears the singleton scene-map instance when this component is disabled.
        /// </summary>
        protected override void OnDisabled() {
            ClearSingletonInstance();
            base.OnDisabled();
        }

        /// <summary>
        /// Registers this instance as the strict singleton component.
        /// </summary>
        void RegisterSingletonInstance() {
            if (Instance == null) {
                Instance = this;
                return;
            }

            if (!ReferenceEquals(Instance, this)) {
                throw new InvalidOperationException("Only one active SceneMapComponent may exist at a time.");
            }
        }

        /// <summary>
        /// Clears the strict singleton component when this instance owns it.
        /// </summary>
        void ClearSingletonInstance() {
            if (ReferenceEquals(Instance, this)) {
                Instance = null;
            }
        }
    }
}
```

- [ ] **Step 2: Remove the old Core-owned service**

```csharp
// Core.cs
namespace helengine {
    public sealed class Core {
        internal RuntimeSceneLoadService SceneLoadService { get; }

        public SceneManager SceneManager { get; private set; }

        void InitializeManagers(ContentManager contentManager) {
            SceneManager = CreateSceneManager(contentManager, InitializationOptions.SceneCatalog);
        }
    }
}
```

```text
Delete file:
C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneMapService.cs
```

- [ ] **Step 3: Run the singleton scene-map tests to verify they pass**

Run: `dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" --no-restore -v minimal`

Expected: PASS

- [ ] **Step 4: Commit the singleton implementation**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/components/SceneMapComponent.cs engine/helengine.core/Core.cs
git -C C:\dev\helworks\helengine rm engine/helengine.core/scene/runtime/SceneMapService.cs
git -C C:\dev\helworks\helengine commit -m "refactor: replace scene map service with singleton component"
```

### Task 3: Route Shared Menu Scene Loads Through SceneMapComponent

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\MenuComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\DemoDiscReturnToMenuRuntimeComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Add failing tests that prove menu scene loads use remapped scene ids**

```csharp
[Test]
public void Update_WhenReturnToMenuIsTriggeredAndMappingExists_LoadsMappedSceneId() {
    WriteSceneAsset("cooked/scenes/DemoDiscMainMenu.hasset", 1u);
    WriteSceneAsset("cooked/scenes/DemoDiscMainMenuDs.hasset", 2u);
    InputTestBackend inputBackend = new InputTestBackend();
    inputBackend.SetGamepadButtonPressed(0, InputGamepadButton.East, true);
    Core core = CreateRuntimeCoreWithCatalog(
        new RuntimeSceneCatalog(
            new RuntimeSceneCatalogEntry("DemoDiscMainMenu", "cooked/scenes/DemoDiscMainMenu.hasset"),
            new RuntimeSceneCatalogEntry("DemoDiscMainMenuDs", "cooked/scenes/DemoDiscMainMenuDs.hasset")),
        inputBackend);
    SceneMapComponent sceneMap = new SceneMapComponent();
    sceneMap.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");
    CreateRootEntityWithComponent(sceneMap);
    CreateRootEntityWithComponent(new DemoDiscReturnToMenuRuntimeComponent());

    core.Update(1d / 60d);

    Assert.True(core.SceneManager.IsSceneLoaded("DemoDiscMainMenuDs"));
    Assert.False(core.SceneManager.IsSceneLoaded("DemoDiscMainMenu"));
}

[Test]
public void MenuItemActivate_WhenTargetSceneIsMapped_LoadsMappedSceneId() {
    WriteSceneAsset("cooked/scenes/colored_cube_grid.hasset", 1u);
    WriteSceneAsset("cooked/scenes/colored_cube_grid_ds.hasset", 2u);
    Core core = CreateRuntimeCoreWithCatalog(
        new RuntimeSceneCatalog(
            new RuntimeSceneCatalogEntry("colored_cube_grid", "cooked/scenes/colored_cube_grid.hasset"),
            new RuntimeSceneCatalogEntry("colored_cube_grid_ds", "cooked/scenes/colored_cube_grid_ds.hasset")));
    SceneMapComponent sceneMap = new SceneMapComponent();
    sceneMap.Mappings.Add("colored_cube_grid", "colored_cube_grid_ds");
    CreateRootEntityWithComponent(sceneMap);
    MenuComponent menuComponent = CreateSingleSceneLoadMenu("colored_cube_grid");

    ActivateSelectedMenuItem(menuComponent);

    Assert.True(core.SceneManager.IsSceneLoaded("colored_cube_grid_ds"));
    Assert.False(core.SceneManager.IsSceneLoaded("colored_cube_grid"));
}
```

- [ ] **Step 2: Run the menu remap tests to verify they fail**

Run: `dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests.Update_WhenReturnToMenuIsTriggeredAndMappingExists_LoadsMappedSceneId|FullyQualifiedName~SceneMapServiceTests.MenuItemActivate_WhenTargetSceneIsMapped_LoadsMappedSceneId" --no-restore -v minimal`

Expected: FAIL because menu code still loads raw scene ids without calling `SceneMapComponent.ResolveSceneId(...)`.

- [ ] **Step 3: Update both menu call sites to resolve scene ids**

```csharp
// MenuComponent.cs
void LoadScene(string sceneId) {
    if (string.IsNullOrWhiteSpace(sceneId)) {
        throw new ArgumentException("Scene id is required.", nameof(sceneId));
    }
    if (Core.Instance.SceneManager == null) {
        throw new InvalidOperationException("Core scene manager must be initialized before menu scene loading can occur.");
    }

    string resolvedSceneId = SceneMapComponent.ResolveSceneId(sceneId);
    Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single);
}
```

```csharp
// DemoDiscReturnToMenuRuntimeComponent.cs
public override void Update() {
    InputSystem inputSystem = Core.Instance.Input;
    bool wasReturnPressed = WasKeyboardReturnPressed(inputSystem)
        || WasGamepadReturnPressed(inputSystem);

    if (wasReturnPressed) {
        string resolvedSceneId = SceneMapComponent.ResolveSceneId(MainMenuSceneId);
        Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single);
    }
}
```

- [ ] **Step 4: Run the menu remap tests to verify they pass**

Run: `dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests.Update_WhenReturnToMenuIsTriggeredAndMappingExists_LoadsMappedSceneId|FullyQualifiedName~SceneMapServiceTests.MenuItemActivate_WhenTargetSceneIsMapped_LoadsMappedSceneId" --no-restore -v minimal`

Expected: PASS

- [ ] **Step 5: Commit the shared menu integration**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/components/2d/menu/MenuComponent.cs engine/helengine.core/components/2d/menu/DemoDiscReturnToMenuRuntimeComponent.cs engine/helengine.editor.tests/serialization/scene/SceneMapServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: route shared menu scene loads through scene map singleton"
```

### Task 4: Update City Return-To-Menu Gameplay To Use The Same Helper

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`

- [ ] **Step 1: Update the city gameplay return component to resolve through SceneMapComponent**

```csharp
namespace city.menu {
    /// <summary>
    /// Returns the active demo-disc scene to the main menu when the temporary back bind is pressed.
    /// </summary>
    public sealed class DemoDiscReturnToMenuComponent : UpdateComponent {
        /// <summary>
        /// Stable runtime scene id used by the demo-disc main menu.
        /// </summary>
        public const string MainMenuSceneId = "DemoDiscMainMenu";

        /// <summary>
        /// Performs per-frame input polling for the demo-disc return bind.
        /// </summary>
        public override void Update() {
            InputSystem inputSystem = Core.Instance.Input;
            bool wasReturnPressed = WasKeyboardReturnPressed(inputSystem)
                || WasGamepadReturnPressed(inputSystem);

            if (wasReturnPressed) {
                string resolvedSceneId = SceneMapComponent.ResolveSceneId(MainMenuSceneId);
                Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single);
            }
        }
    }
}
```

- [ ] **Step 2: Verify the city source now uses the shared helper**

Run: `rg -n "SceneMapComponent\\.ResolveSceneId|LoadScene\\(MainMenuSceneId" C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`

Expected:
- one hit for `SceneMapComponent.ResolveSceneId`
- one hit for `LoadScene(resolvedSceneId, SceneLoadMode.Single)`
- no direct `LoadScene(MainMenuSceneId, SceneLoadMode.Single)`

- [ ] **Step 3: Commit the city gameplay update**

```bash
git -C C:\dev\helprojs\city add assets/codebase/menu/DemoDiscReturnToMenuComponent.cs
git -C C:\dev\helprojs\city commit -m "feat: resolve return-to-menu scene ids through scene map singleton"
```

### Task 5: Run Full Shared Verification And DS Build Verification

**Files:**
- Verify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`
- Verify: `C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj`
- Verify: `C:\dev\helprojs\city\project.heproj`

- [ ] **Step 1: Run the focused shared test slice**

Run: `dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests|FullyQualifiedName~SceneMapComponentPersistenceDescriptorTests" --no-restore -v minimal`

Expected: PASS

- [ ] **Step 2: Run the editor-driven DS build using the real city project**

Run: `rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds`

Expected: `Build completed for platform 'ds'`

- [ ] **Step 3: Confirm the DS return path now uses the remapped menu id**

Run: `rg -a -n "DemoDiscMainMenuDs|SceneMapComponent" C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

Expected:
- staged DS output references `DemoDiscMainMenuDs`
- the runtime contains `SceneMapComponent` support

- [ ] **Step 4: Commit the final integrated verification state**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/components/SceneMapComponent.cs engine/helengine.core/components/2d/menu/MenuComponent.cs engine/helengine.core/components/2d/menu/DemoDiscReturnToMenuRuntimeComponent.cs engine/helengine.core/Core.cs engine/helengine.editor.tests/serialization/scene/SceneMapServiceTests.cs engine/helengine.editor.tests/serialization/scene/SceneMapComponentPersistenceDescriptorTests.cs
git -C C:\dev\helworks\helengine commit -m "refactor: make scene map component the singleton menu resolver"
```

```bash
git -C C:\dev\helprojs\city add assets/codebase/menu/DemoDiscReturnToMenuComponent.cs
git -C C:\dev\helprojs\city commit -m "feat: use singleton scene map resolution for menu return"
```

---

## Self-Review

### Spec coverage

- remove `SceneMapService`: covered in Task 2
- remove `Core.SceneMapService`: covered in Task 2
- strict singleton `SceneMapComponent`: covered in Tasks 1 and 2
- all menu loads use singleton helper: covered in Tasks 3 and 4
- duplicate instances crash: covered in Tasks 1 and 2
- DS menu return resolves `DemoDiscMainMenu -> DemoDiscMainMenuDs`: covered in Tasks 3, 4, and 5

### Placeholder scan

- no `TODO` or `TBD`
- all steps include explicit files, commands, and code

### Type consistency

- runtime helper name is consistently `SceneMapComponent.ResolveSceneId(...)`
- singleton field name is consistently `SceneMapComponent.Instance`
- all menu loads still terminate at `SceneManager.LoadScene(...)`

