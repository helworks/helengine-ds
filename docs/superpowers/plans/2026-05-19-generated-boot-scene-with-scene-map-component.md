# Generated Boot Scene With SceneMapComponent Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a generated boot scene that contains `SceneMapComponent`, lets that component optionally load the first runtime scene through `InitialSceneId`, and routes DS startup through the generated boot scene instead of DS-specific startup-scene path rewriting.

**Architecture:** Extend `SceneMapComponent` into a strict singleton helper with optional one-shot startup redirect behavior, then add editor-side boot-scene generation beside the existing generated menu scene flow. After the shared runtime and editor manifest flow are correct, remove the DS startup-scene rewrite dependence and verify DS now boots through the generated boot scene.

**Tech Stack:** C#, xUnit, shared `helengine.core`, `helengine.editor`, Nintendo DS builder tests, native generated-core staging path

---

### Task 1: Extend SceneMapComponent Into A Singleton Startup Redirect Helper

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\SceneMapComponent.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Write the failing singleton and startup-redirect tests**

Add tests in `SceneMapServiceTests.cs` that define the new runtime contract:

```csharp
[Fact]
public void ResolveSceneId_WhenNoSingletonExists_ReturnsOriginalSceneId() {
    string resolvedSceneId = SceneMapComponent.ResolveSceneId("DemoDiscMainMenu");
    Assert.Equal("DemoDiscMainMenu", resolvedSceneId);
}

[Fact]
public void ComponentAdded_WhenSecondSceneMapComponentAppears_ThrowsInvalidOperationException() {
    SceneMapComponent first = new SceneMapComponent();
    SceneMapComponent second = new SceneMapComponent();
    Entity rootEntity = CreateRootEntity();

    rootEntity.AddComponent(first);
    InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => rootEntity.AddComponent(second));

    Assert.Contains("Only one active SceneMapComponent may exist at a time.", exception.Message, StringComparison.Ordinal);
}

[Fact]
public void Update_WhenInitialSceneIdIsPresent_LoadsResolvedSceneIdOnce() {
    WriteSceneAsset("cooked/scenes/DemoDiscMainMenu.hasset", 1u);
    WriteSceneAsset("cooked/scenes/DemoDiscMainMenuDs.hasset", 2u);

    Core core = CreateCore(CreateSceneCatalog(
        new RuntimeSceneCatalogEntry("DemoDiscMainMenu", "cooked/scenes/DemoDiscMainMenu.hasset"),
        new RuntimeSceneCatalogEntry("DemoDiscMainMenuDs", "cooked/scenes/DemoDiscMainMenuDs.hasset")));

    SceneMapComponent sceneMapComponent = new SceneMapComponent {
        InitialSceneId = "DemoDiscMainMenu"
    };
    sceneMapComponent.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");
    AddLoadedScene(core.SceneManager, "Scenes/GeneratedBoot.hasset", CreateRootEntityWithComponent(sceneMapComponent));

    core.Update(1d / 60d);
    core.Update(1d / 60d);

    Assert.True(core.SceneManager.IsSceneLoaded("DemoDiscMainMenuDs"));
    Assert.False(core.SceneManager.IsSceneLoaded("DemoDiscMainMenu"));
}
```

- [ ] **Step 2: Run the focused shared scene-map tests and confirm failure**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" --no-restore -v minimal
```

Expected: FAIL because `SceneMapComponent` does not yet expose `ResolveSceneId`, `Instance`, or `InitialSceneId`.

- [ ] **Step 3: Implement the singleton and startup redirect behavior**

Update `SceneMapComponent.cs` so it owns the helper behavior directly:

```csharp
namespace helengine {
    /// <summary>
    /// Stores authored scene-id remapping entries and optionally redirects startup into one mapped logical scene id.
    /// </summary>
    public sealed class SceneMapComponent : Component {
        /// <summary>
        /// Current cooked payload version used by runtime scene persistence.
        /// </summary>
        public const byte CurrentVersion = 2;

        /// <summary>
        /// Singleton instance for the active scene-map component.
        /// </summary>
        public static SceneMapComponent Instance { get; private set; }

        /// <summary>
        /// Tracks whether the optional startup redirect already executed for this process lifetime.
        /// </summary>
        static bool StartupSceneWasRequested;

        /// <summary>
        /// Initializes one empty scene-map component.
        /// </summary>
        public SceneMapComponent() {
            Mappings = new Dictionary<string, string>(StringComparer.Ordinal);
            InitialSceneId = string.Empty;
        }

        /// <summary>
        /// Gets or sets the logical startup scene id that should be loaded once after the singleton registers.
        /// </summary>
        [EditorPropertyDisplayName("Initial Scene Id")]
        [EditorPropertyOrder(1)]
        public string InitialSceneId { get; set; }

        /// <summary>
        /// Gets the authored mapping entries keyed by logical source scene id.
        /// </summary>
        [EditorPropertyDisplayName("Scene Mappings")]
        [EditorPropertyOrder(0)]
        public Dictionary<string, string> Mappings { get; }

        /// <summary>
        /// Resolves one logical scene id through the active singleton when present.
        /// </summary>
        public static string ResolveSceneId(string sceneId) {
            if (string.IsNullOrWhiteSpace(sceneId)) {
                throw new ArgumentException("Scene id must be provided.", nameof(sceneId));
            }
            if (Instance == null) {
                return sceneId;
            }
            if (Instance.Mappings.TryGetValue(sceneId, out string mappedSceneId) && !string.IsNullOrWhiteSpace(mappedSceneId)) {
                return mappedSceneId;
            }

            return sceneId;
        }

        /// <summary>
        /// Registers this component as the singleton when attached to an entity.
        /// </summary>
        protected override void ComponentAdded(Entity entity) {
            base.ComponentAdded(entity);
            RegisterSingleton();
            RequestInitialSceneLoad();
        }

        /// <summary>
        /// Clears the singleton when this component is detached from its entity.
        /// </summary>
        protected override void ComponentRemoved(Entity entity) {
            ClearSingletonIfOwned();
            base.ComponentRemoved(entity);
        }

        /// <summary>
        /// Releases the singleton ownership when this component is disposed.
        /// </summary>
        public override void Dispose() {
            ClearSingletonIfOwned();
            base.Dispose();
        }
    }
}
```

Also add private helpers inside the same type:

```csharp
void RegisterSingleton() {
    if (Instance == null) {
        Instance = this;
        return;
    } else if (!ReferenceEquals(Instance, this)) {
        throw new InvalidOperationException("Only one active SceneMapComponent may exist at a time.");
    }
}

void RequestInitialSceneLoad() {
    if (StartupSceneWasRequested) {
        return;
    } else if (string.IsNullOrWhiteSpace(InitialSceneId)) {
        return;
    }

    string resolvedSceneId = ResolveSceneId(InitialSceneId);
    StartupSceneWasRequested = true;
    Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single);
}

void ClearSingletonIfOwned() {
    if (ReferenceEquals(Instance, this)) {
        Instance = null;
    }
}
```

- [ ] **Step 4: Update the tests for the new singleton reset mechanics**

Replace the old `Core.SceneMapService` assumptions in `SceneMapServiceTests.cs` with singleton reset helpers:

```csharp
static void ResetSceneMapComponentSingleton() {
    PropertyInfo instanceProperty = typeof(SceneMapComponent).GetProperty(nameof(SceneMapComponent.Instance), BindingFlags.Static | BindingFlags.Public);
    MethodInfo setter = instanceProperty.GetSetMethod(true);
    setter.Invoke(null, [null]);

    FieldInfo startupField = typeof(SceneMapComponent).GetField("StartupSceneWasRequested", BindingFlags.Static | BindingFlags.NonPublic);
    startupField.SetValue(null, false);
}
```

Call the reset helper from constructor and `Dispose()` so tests are isolated.

- [ ] **Step 5: Run the focused shared scene-map tests and confirm pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" --no-restore -v minimal
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add C:/dev/helworks/helengine/engine/helengine.core/components/SceneMapComponent.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/SceneMapServiceTests.cs
git commit -m "feat: add scene map singleton startup redirect"
```

### Task 2: Persist InitialSceneId Through Scene Serialization

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneMapComponentPersistenceDescriptor.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Write the failing persistence test for InitialSceneId**

Add a serialization round-trip test:

```csharp
[Fact]
public void SerializeAndDeserialize_WhenInitialSceneIdIsPresent_RoundTripsInitialSceneIdAndMappings() {
    SceneMapComponent component = new SceneMapComponent {
        InitialSceneId = "DemoDiscMainMenu"
    };
    component.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");

    SceneMapComponentPersistenceDescriptor descriptor = new SceneMapComponentPersistenceDescriptor();
    SceneComponentAssetRecord record = descriptor.SerializeComponent(component, 0, null);
    SceneMapComponent deserialized = Assert.IsType<SceneMapComponent>(descriptor.DeserializeComponent(record, null, null));

    Assert.Equal("DemoDiscMainMenu", deserialized.InitialSceneId);
    Assert.Equal("DemoDiscMainMenuDs", deserialized.Mappings["DemoDiscMainMenu"]);
}
```

- [ ] **Step 2: Run the focused scene-map tests and confirm failure**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" --no-restore -v minimal
```

Expected: FAIL because `InitialSceneId` is not yet serialized.

- [ ] **Step 3: Extend SceneMapComponent persistence to version 2**

Update `SceneMapComponentPersistenceDescriptor.cs` so both tagged and cooked runtime payloads include `InitialSceneId`:

```csharp
const byte InitialSceneIdVersion = 2;

public SceneComponentAssetRecord SerializeComponent(Component component, uint componentIndex, Asset[] assetReferences) {
    SceneMapComponent sceneMapComponent = component as SceneMapComponent
        ?? throw new InvalidOperationException("Scene map persistence requires a SceneMapComponent.");

    EditorTaggedSceneComponentFieldWriter writer = new EditorTaggedSceneComponentFieldWriter();
    writer.WriteField("InitialSceneId", fieldWriter => fieldWriter.WriteString(sceneMapComponent.InitialSceneId ?? string.Empty));
    writer.WriteField("Mappings", fieldWriter => WriteMappings(fieldWriter, sceneMapComponent.Mappings));

    return new SceneComponentAssetRecord {
        ComponentTypeId = ComponentTypeId,
        ComponentIndex = componentIndex,
        Payload = writer.BuildPayload(InitialSceneIdVersion)
    };
}
```

During deserialize, accept both versions:

```csharp
if (record.Version >= 2) {
    component.InitialSceneId = reader.ReadString();
} else {
    component.InitialSceneId = string.Empty;
}
```

- [ ] **Step 4: Run the focused scene-map tests and confirm pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" --no-restore -v minimal
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor/serialization/scene/SceneMapComponentPersistenceDescriptor.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/SceneMapServiceTests.cs
git commit -m "feat: persist scene map initial scene id"
```

### Task 3: Generate A Minimal Boot Scene In The Editor

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\GeneratedBootSceneAssetFactory.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedBootScenePreparationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedMenuScenePreparationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformBuildGraphRunner.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedMenuScenePreparationServiceTests.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedBootScenePreparationServiceTests.cs`

- [ ] **Step 1: Write the failing boot-scene generation tests**

Add a new editor test file `EditorGeneratedBootScenePreparationServiceTests.cs`:

```csharp
namespace helengine.editor.tests.managers.project;

public sealed class EditorGeneratedBootScenePreparationServiceTests : IDisposable {
    [Fact]
    public void EnsurePrepared_WhenNintendoDsMenuSceneIsSelected_WritesBootSceneWithSceneMapComponent() {
        ScriptTypeResolver resolver = new ScriptTypeResolver();
        resolver.Register("gameplay", typeof(TestMenuDefinitionProvider).Assembly);
        EditorMenuSceneRegenerationService regenerationService = new EditorMenuSceneRegenerationService(ProjectRootPath, resolver);
        regenerationService.Regenerate("Scenes/DemoDiscMainMenu.helen", typeof(TestMenuDefinitionProvider).FullName + ", gameplay");

        EditorGeneratedBootScenePreparationService service = new EditorGeneratedBootScenePreparationService(ProjectRootPath);
        service.EnsurePrepared("ds", [PlatformMenuSceneResolver.NintendoDsMainMenuSceneId]);

        string scenePath = Path.Combine(ProjectRootPath, "assets", "Scenes", "GeneratedBootScene.helen");
        Assert.True(File.Exists(scenePath));

        using FileStream stream = File.OpenRead(scenePath);
        SceneAsset sceneAsset = Assert.IsType<SceneAsset>(AssetSerializer.Deserialize(stream));
        Assert.Single(sceneAsset.RootEntities);
    }
}
```

Add one assertion to `EditorGeneratedMenuScenePreparationServiceTests.cs` that `EnsurePrepared(...)` also creates the boot scene when DS menu generation runs.

- [ ] **Step 2: Run the focused preparation tests and confirm failure**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedMenuScenePreparationServiceTests|FullyQualifiedName~EditorGeneratedBootScenePreparationServiceTests" --no-restore -v minimal
```

Expected: FAIL because there is no boot-scene factory or preparation service yet.

- [ ] **Step 3: Create the generated boot-scene factory**

Create `GeneratedBootSceneAssetFactory.cs`:

```csharp
namespace helengine.editor {
    /// <summary>
    /// Builds a minimal generated boot scene that contains only one root entity with one SceneMapComponent.
    /// </summary>
    public sealed class GeneratedBootSceneAssetFactory {
        /// <summary>
        /// Descriptor used to serialize automatically reflected component payloads.
        /// </summary>
        readonly AutomaticScriptComponentPersistenceDescriptor AutomaticDescriptor;

        /// <summary>
        /// Initializes the generated boot-scene factory.
        /// </summary>
        public GeneratedBootSceneAssetFactory() {
            AutomaticDescriptor = new AutomaticScriptComponentPersistenceDescriptor(new ScriptComponentReflectionSchemaBuilder());
        }

        /// <summary>
        /// Builds one generated boot scene asset.
        /// </summary>
        public SceneAsset BuildSceneAsset(string sceneId, string initialSceneId, IReadOnlyDictionary<string, string> mappings) {
            SceneMapComponent sceneMapComponent = new SceneMapComponent {
                InitialSceneId = initialSceneId
            };

            foreach ((string sourceSceneId, string targetSceneId) in mappings) {
                sceneMapComponent.Mappings.Add(sourceSceneId, targetSceneId);
            }

            return new SceneAsset {
                Id = sceneId,
                RootEntities = [
                    new SceneEntityAsset {
                        Id = 1,
                        Name = "GeneratedBootSceneRoot",
                        LocalPosition = float3.Zero,
                        LocalScale = float3.One,
                        LocalOrientation = float4.Identity,
                        Components = [
                            AutomaticDescriptor.SerializeComponent(sceneMapComponent, 0, null)
                        ],
                        Children = Array.Empty<SceneEntityAsset>()
                    }
                ]
            };
        }
    }
}
```

- [ ] **Step 4: Create the generated boot-scene preparation service**

Create `EditorGeneratedBootScenePreparationService.cs`:

```csharp
namespace helengine.editor {
    /// <summary>
    /// Ensures generated boot scenes exist for platforms that route startup through SceneMapComponent.
    /// </summary>
    public sealed class EditorGeneratedBootScenePreparationService {
        /// <summary>
        /// Stable generated boot-scene id.
        /// </summary>
        public const string GeneratedBootSceneId = "GeneratedBootScene";

        readonly string ProjectRootPath;
        readonly GeneratedBootSceneAssetFactory BootSceneFactory;

        public EditorGeneratedBootScenePreparationService(string projectRootPath) {
            ProjectRootPath = string.IsNullOrWhiteSpace(projectRootPath)
                ? throw new ArgumentException("Project root path must be provided.", nameof(projectRootPath))
                : Path.GetFullPath(projectRootPath);
            BootSceneFactory = new GeneratedBootSceneAssetFactory();
        }

        public void EnsurePrepared(string platformId, IReadOnlyList<string> sceneIds) {
            if (!string.Equals(platformId, "ds", StringComparison.OrdinalIgnoreCase)) {
                return;
            }
            if (!ContainsSceneId(sceneIds, PlatformMenuSceneResolver.NintendoDsMainMenuSceneId)) {
                return;
            }

            SceneAsset sceneAsset = BootSceneFactory.BuildSceneAsset(
                GeneratedBootSceneId,
                PlatformMenuSceneResolver.DesktopMainMenuSceneId,
                new Dictionary<string, string>(StringComparer.Ordinal) {
                    { PlatformMenuSceneResolver.DesktopMainMenuSceneId, PlatformMenuSceneResolver.NintendoDsMainMenuSceneId }
                });

            string scenePath = Path.Combine(ProjectRootPath, "assets", "Scenes", GeneratedBootSceneId + ".helen");
            Directory.CreateDirectory(Path.GetDirectoryName(scenePath));
            using FileStream stream = new FileStream(scenePath, FileMode.Create, FileAccess.Write, FileShare.None);
            AssetSerializer.Serialize(stream, sceneAsset);
        }
    }
}
```

- [ ] **Step 5: Wire boot-scene preparation into the existing build graph**

Update `EditorPlatformBuildGraphRunner.cs` and `EditorGeneratedMenuScenePreparationService.cs` so generated menu and boot-scene preparation happen before selected-scene resolution:

```csharp
readonly EditorGeneratedBootScenePreparationService GeneratedBootScenePreparationService;

GeneratedBootScenePreparationService = new EditorGeneratedBootScenePreparationService(ProjectRootPath);
```

Then before scene cooking:

```csharp
GeneratedMenuScenePreparationService.EnsurePrepared(queueItem.SelectedSceneIds ?? []);
GeneratedBootScenePreparationService.EnsurePrepared(queueItem.PlatformId, queueItem.SelectedSceneIds ?? []);
```

- [ ] **Step 6: Run the focused preparation tests and confirm pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedMenuScenePreparationServiceTests|FullyQualifiedName~EditorGeneratedBootScenePreparationServiceTests" --no-restore -v minimal
```

Expected: PASS

- [ ] **Step 7: Commit**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor/managers/menu/GeneratedBootSceneAssetFactory.cs C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorGeneratedBootScenePreparationService.cs C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorGeneratedMenuScenePreparationService.cs C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformBuildGraphRunner.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorGeneratedMenuScenePreparationServiceTests.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorGeneratedBootScenePreparationServiceTests.cs
git commit -m "feat: generate boot scene with scene map component"
```

### Task 4: Route Startup To The Generated Boot Scene

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorBuildQueueItemFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\content\PlatformMenuSceneResolver.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorBuildQueueItemFactoryTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformBuildGraphRunnerTests.cs`

- [ ] **Step 1: Write the failing startup-scene override tests**

Add assertions in `EditorBuildQueueItemFactoryTests.cs`:

```csharp
[Fact]
public void Create_WhenPlatformIsNintendoDs_InsertsGeneratedBootSceneFirst() {
    EditorBuildQueueItemFactory factory = CreateFactory(["DemoDiscMainMenu", "DemoDiscMainMenuDs"]);

    EditorBuildQueueItemDocument queueItem = factory.Create(
        CreatePlatformConfig("ds", ["DemoDiscMainMenuDs"]),
        CreateSelectionModel(),
        OutputDirectoryPath);

    Assert.Equal("GeneratedBootScene", Assert.Single(queueItem.SelectedSceneIds.Take(1)));
    Assert.Contains("DemoDiscMainMenuDs", queueItem.SelectedSceneIds, StringComparer.Ordinal);
}
```

Add one build-graph test that verifies the cooked startup scene id becomes the generated boot scene.

- [ ] **Step 2: Run the focused build-queue tests and confirm failure**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorBuildQueueItemFactoryTests|FullyQualifiedName~EditorPlatformBuildGraphRunnerTests" --no-restore -v minimal
```

Expected: FAIL because startup still points directly to `DemoDiscMainMenuDs`.

- [ ] **Step 3: Introduce a shared generated boot-scene id and route DS startup through it**

Update `PlatformMenuSceneResolver.cs`:

```csharp
public static class PlatformMenuSceneResolver {
    public const string DesktopMainMenuSceneId = "DemoDiscMainMenu";
    public const string NintendoDsMainMenuSceneId = "DemoDiscMainMenuDs";
    public const string GeneratedBootSceneId = "GeneratedBootScene";
}
```

Update `EditorBuildQueueItemFactory.cs`:

```csharp
const string NintendoDsStartupSceneId = PlatformMenuSceneResolver.GeneratedBootSceneId;
```

Leave the rest of the scene selection unchanged so the DS menu scene still cooks and stages alongside the boot scene.

- [ ] **Step 4: Run the focused build-queue tests and confirm pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorBuildQueueItemFactoryTests|FullyQualifiedName~EditorPlatformBuildGraphRunnerTests" --no-restore -v minimal
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorBuildQueueItemFactory.cs C:/dev/helworks/helengine/engine/helengine.core/content/PlatformMenuSceneResolver.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorBuildQueueItemFactoryTests.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformBuildGraphRunnerTests.cs
git commit -m "feat: route DS startup through generated boot scene"
```

### Task 5: Remove DS Startup Rewrite Dependence And Prove Boot-Scene Startup

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsGeneratedCoreStager.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsStartupSceneIds.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsGeneratedCoreStagerTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Write the failing DS startup-manifest tests**

Add one DS builder test that verifies the cooked manifest already contains the generated boot scene, and one stager test that verifies the stager no longer rewrites the startup path to `DemoDiscMainMenuDs`:

```csharp
[Fact]
public void Stage_whenRuntimeStartupManifestAlreadyPointsToGeneratedBootScene_preservesManifestPath() {
    string manifestSource =
        "static const char kRuntimeStartupSceneRelativePath[] = \"cooked/scenes/GeneratedBootScene.hasset\";\n"
        + "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }";

    File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), manifestSource);

    NintendoDsGeneratedCoreStager stager = new();
    stager.Stage(sourceRootPath, destinationRootPath);

    string stagedManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "runtime_startup_manifest.cpp"));
    Assert.Contains("GeneratedBootScene.hasset", stagedManifestSource, StringComparison.Ordinal);
    Assert.DoesNotContain("DemoDiscMainMenuDs.hasset", stagedManifestSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused DS builder tests and confirm failure**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore -v minimal
```

Expected: FAIL because the DS stager still rewrites startup to `DemoDiscMainMenuDs`.

- [ ] **Step 3: Remove the DS-specific startup rewrite and update DS constants**

In `NintendoDsGeneratedCoreStager.cs`, remove the startup-scene rewrite pieces:

```csharp
static void ApplyNintendoDsRuntimeFixups(string destinationRootPath) {
    NormalizePlatformConfiguration(destinationRootPath);
    ForceShadersDisabled(destinationRootPath);
    ReplaceHlslShaderBindingParserWithStub(destinationRootPath);
    TrimRuntimeSceneResolverHeaderIncludes(destinationRootPath);
    RemoveLegacyBitConverterIncludes(destinationRootPath);
    StripRuntimeShaderPackageDependency(destinationRootPath);
    NormalizeRuntimeTextureOwnedAssetSeams(destinationRootPath);
}
```

Delete:

```csharp
static readonly Regex RuntimeStartupScenePathPattern = ...
static readonly Regex RuntimeStartupSceneConstantPattern = ...
static void RewriteRuntimeStartupScenePath(string destinationRootPath) ...
static string RewriteRuntimeStartupManifest(string source) ...
```

Update `NintendoDsStartupSceneIds.cs` so the DS startup scene id and cooked relative path represent the generated boot scene:

```csharp
public static class NintendoDsStartupSceneIds {
    public const string DemoDiscMainMenuSceneId = PlatformMenuSceneResolver.GeneratedBootSceneId;
    public const string DemoDiscMainMenuCookedRelativePath = "cooked/scenes/GeneratedBootScene.hasset";
}
```

- [ ] **Step 4: Run the focused DS builder tests and confirm pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore -v minimal
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add C:/dev/helworks/helengine-ds/builder/NintendoDsGeneratedCoreStager.cs C:/dev/helworks/helengine-ds/builder/NintendoDsStartupSceneIds.cs C:/dev/helworks/helengine-ds/builder.tests/NintendoDsGeneratedCoreStagerTests.cs C:/dev/helworks/helengine-ds/builder.tests/NintendoDsPlatformAssetBuilderTests.cs
git commit -m "refactor: remove DS startup scene rewrite dependency"
```

### Task 6: Verify End-To-End DS Boot Routing Through The Generated Boot Scene

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Write the failing end-to-end return-to-menu test**

Add one test that proves the logical menu scene id resolves through `SceneMapComponent` after startup boot-scene registration:

```csharp
[Fact]
public void Update_WhenReturnToMenuIsTriggeredAfterBootSceneRegistration_LoadsMappedDsMenuScene() {
    WriteSceneAsset("cooked/scenes/GeneratedBootScene.hasset", 1u);
    WriteSceneAsset("cooked/scenes/DemoDiscMainMenuDs.hasset", 2u);

    TestInputBackend inputBackend = new TestInputBackend();
    inputBackend.Gamepads = [CreatePressedGamepadState(InputGamepadButton.Select)];
    inputBackend.GamepadCount = 1;

    Core core = CreateCore(CreateSceneCatalog(
        new RuntimeSceneCatalogEntry("GeneratedBootScene", "cooked/scenes/GeneratedBootScene.hasset"),
        new RuntimeSceneCatalogEntry("DemoDiscMainMenuDs", "cooked/scenes/DemoDiscMainMenuDs.hasset")), inputBackend);

    SceneMapComponent sceneMapComponent = new SceneMapComponent {
        InitialSceneId = "DemoDiscMainMenu"
    };
    sceneMapComponent.Mappings.Add("DemoDiscMainMenu", "DemoDiscMainMenuDs");
    AddLoadedScene(core.SceneManager, "GeneratedBootScene", CreateRootEntityWithComponent(sceneMapComponent));
    CreateRootEntityWithComponent(new DemoDiscReturnToMenuRuntimeComponent());

    core.Update(1d / 60d);

    Assert.True(core.SceneManager.IsSceneLoaded("DemoDiscMainMenuDs"));
}
```

- [ ] **Step 2: Update the real city return component to use the helper directly**

Ensure `DemoDiscReturnToMenuComponent.cs` in the city project uses the shared helper and not a hardcoded scene id load:

```csharp
string resolvedSceneId = SceneMapComponent.ResolveSceneId(MainMenuSceneId);
Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single);
```

- [ ] **Step 3: Run the focused shared + DS tests and confirm pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests|FullyQualifiedName~EditorGeneratedMenuScenePreparationServiceTests|FullyQualifiedName~EditorGeneratedBootScenePreparationServiceTests|FullyQualifiedName~EditorBuildQueueItemFactoryTests" --no-restore -v minimal
```

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore -v minimal
```

Expected: PASS

- [ ] **Step 4: Build the DS ROM and inspect the startup scene path**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected:

```text
Build completed for platform 'ds': C:\tmp\helengine-ds-city-cube-project\output\ds
```

Then inspect the ROM or staged generated-core manifest:

```powershell
rg -a -n "GeneratedBootScene|DemoDiscMainMenuDs" C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds
```

Expected:
- startup manifest contains `GeneratedBootScene`
- DS menu scene id `DemoDiscMainMenuDs` is still present as a remap target

- [ ] **Step 5: Launch the ROM and verify behavior manually**

Run:

```powershell
& "C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe" "C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds"
```

Manual expected behavior:
- boot reaches the DS menu through the boot scene
- return-to-menu from a gameplay scene reaches `DemoDiscMainMenuDs`
- duplicate `SceneMapComponent` setup crashes immediately if intentionally authored

- [ ] **Step 6: Commit**

```bash
git add C:/dev/helprojs/city/assets/codebase/menu/DemoDiscReturnToMenuComponent.cs C:/dev/helworks/helengine/engine/helengine.editor.tests/serialization/scene/SceneMapServiceTests.cs C:/dev/helworks/helengine-ds/builder.tests/NintendoDsPlatformAssetBuilderTests.cs
git commit -m "feat: boot through generated scene map boot scene"
```
