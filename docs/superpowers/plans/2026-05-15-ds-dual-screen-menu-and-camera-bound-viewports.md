# DS Dual-Screen Menu And Camera-Bound Viewports Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the split viewport/reference-fit contract with a shared camera-bindable `ViewportComponent`, then generate and route a Nintendo DS-specific dual-screen menu scene that uses top-screen branding and bottom-screen interactive menu content.

**Architecture:** The work lands in three vertical slices. First, unify viewport area, optional scaling, and optional camera targeting under the shared engine `ViewportComponent` while preserving implicit compatibility for existing scenes. Second, add a dedicated DS menu scene generation and routing seam so DS startup and return-to-menu resolve a DS-owned scene instead of the desktop menu. Third, update the DS backend to interpret vertically stacked dual-screen viewports and render two bound cameras in one frame.

**Tech Stack:** C#, .NET 9, xUnit, generated-core C++, Nintendo DS libnds runtime, RTK tooling

---

## File Structure

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\ViewportComponent.cs`
  Expands the shared authored viewport contract to include scaling state and explicit camera binding.
- Create: `C:\dev\helworks\helengine\engine\helengine.core\components\ViewportLayoutSnapshot.cs`
  Holds authored subtree layout state now that scaling moves under `ViewportComponent`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\ReferenceCanvasFitComponent.cs`
  Reduced to a compatibility wrapper or removed from new authoring paths after the viewport-owned scaling logic exists.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\ReferenceCanvasFitSnapshot.cs`
  Reused temporarily or retired once the viewport-owned snapshot path is in place.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\utils\CameraViewportResolver.cs`
  Extends shared viewport resolution rules to support DS stacked normalized space through backend-specific target dimensions.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\MenuComponent.cs`
  Routes return-to-menu through a shared platform-aware menu scene resolver seam.
- Create: `C:\dev\helworks\helengine\engine\helengine.core\content\PlatformMenuSceneResolver.cs`
  Shared runtime helper for resolving the current platform menu scene id/path.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\DemoMenuSceneAssetFactory.cs`
  Keeps desktop generator behavior correct under the new `ViewportComponent` contract.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\NintendoDsMenuSceneAssetFactory.cs`
  Builds the DS-specific dual-screen menu scene.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\DemoMenuSceneBuildService.cs`
  Gains an explicit DS build path or factory selection seam.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorBuildQueueItemFactory.cs`
  Ensures DS builds select the DS-specific menu scene first.
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsStartupSceneIds.cs`
  Renames or extends DS startup ids to point at the DS-generated menu scene.
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsGeneratedCoreStager.cs`
  Rewrites staged startup manifest references to the DS-owned menu scene.
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformAssetBuilder.cs`
  Validates the DS-owned menu scene is staged.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsBootHost.cpp`
  Detects the new DS menu scene and maintains the correct screen modes.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
  Stops assuming first-camera-only presentation and resolves DS screen routing from viewport-bound cameras.
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp`
  Draws camera-bound 2D content into top or bottom screen targets based on DS viewport mapping.
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\components\ReferenceCanvasFitComponentTests.cs`
  Migrates old scaling coverage onto the new viewport-owned scaling model.
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\ViewportAndAnchorLayoutTests.cs`
  Verifies viewport binding, scaling, and explicit camera targeting behavior.
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\rendering\CameraViewportResolverTests.cs`
  Adds stacked-screen viewport resolution coverage.
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\menu\EditorMenuSceneRegenerationServiceTests.cs`
  Verifies new generator output no longer emits `ReferenceCanvasFitComponent`.
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
  Verifies return-to-menu routes through the platform-aware menu scene resolver.
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsGeneratedCoreStagerTests.cs`
  Verifies DS startup-scene remap targets the DS-owned menu scene.
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`
  Verifies DS staging requires the DS-owned menu scene payload.
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`
  Verifies DS renderer source now resolves viewports/cameras instead of first-camera-only assumptions.

### Task 1: Unify Viewport Scaling And Camera Binding

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.core\components\ViewportLayoutSnapshot.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\ViewportComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\ReferenceCanvasFitComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\components\ReferenceCanvasFitComponentTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\ViewportAndAnchorLayoutTests.cs`

- [ ] **Step 1: Write the failing viewport-owned scaling tests**

```csharp
[Fact]
public void Update_WhenViewportScalingIsEnabled_ScalesTheAuthoredSubtreeUniformly() {
    TestRenderManager3D renderManager = Assert.IsType<TestRenderManager3D>(Core.Instance.RenderManager3D);
    renderManager.OnWindowResize(IntPtr.Zero, 1280, 720);

    Entity menuRoot = CreateEntity(float3.Zero);
    menuRoot.AddComponent(new ViewportComponent {
        BindingMode = ViewportComponent.ScreenBindingMode,
        FixedSize = new int2(1280, 720),
        ScalingMode = ViewportComponent.ReferenceCanvasScalingMode,
        ReferenceWidth = 1280,
        ReferenceHeight = 720
    });

    Entity panelEntity = CreateEntity(new float3(88f, 190f, 0f));
    RoundedRectComponent panelBackground = new RoundedRectComponent {
        Size = new int2(560, 420),
        Radius = 18f,
        BorderThickness = 3f
    };
    panelEntity.AddComponent(panelBackground);
    menuRoot.AddChild(panelEntity);

    renderManager.OnWindowResize(IntPtr.Zero, 640, 480);
    Core.Instance.Update();

    Assert.Equal(new float3(44f, 95f, 0f), panelEntity.LocalPosition);
    Assert.Equal(new int2(280, 210), panelBackground.Size);
}

[Fact]
public void AnchorComponent_WhenViewportBindsToExplicitCamera_UsesTheTargetCameraViewport() {
    Entity leftCameraEntity = CreateEntity(float3.Zero);
    CameraComponent leftCamera = new CameraComponent {
        Viewport = new float4(0f, 0f, 320f, 180f)
    };
    leftCameraEntity.AddComponent(leftCamera);

    Entity rightCameraEntity = CreateEntity(float3.Zero);
    CameraComponent rightCamera = new CameraComponent {
        Viewport = new float4(0f, 0f, 640f, 360f)
    };
    rightCameraEntity.AddComponent(rightCamera);

    Entity viewportEntity = CreateEntity(float3.Zero);
    viewportEntity.AddComponent(new ViewportComponent {
        BindingMode = ViewportComponent.ExplicitCameraBindingMode,
        BoundCameraComponent = rightCamera
    });

    Entity contentEntity = CreateEntity(new float3(520f, 290f, 0f));
    contentEntity.AddComponent(new RoundedRectComponent {
        Size = new int2(100, 50)
    });
    contentEntity.AddComponent(new AnchorComponent());
    viewportEntity.AddChild(contentEntity);

    AnchorComponent anchor = Assert.IsType<AnchorComponent>(Assert.Single(contentEntity.Components, component => component is AnchorComponent));
    anchor.EnableAnchoring(right: true, bottom: true);

    Assert.Equal(new float3(520f, 290f, 0f), contentEntity.LocalPosition);
}
```

- [ ] **Step 2: Run the focused tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~ReferenceCanvasFitComponentTests|FullyQualifiedName~ViewportAndAnchorLayoutTests" --no-restore`

Expected: FAIL with missing `ScalingMode`, `ReferenceWidth`, `ReferenceHeight`, `ExplicitCameraBindingMode`, or `BoundCameraComponent` members on `ViewportComponent`.

- [ ] **Step 3: Add the viewport-owned scaling snapshot type**

```csharp
namespace helengine {
    /// <summary>
    /// Stores the authored layout state for one viewport-scaled entity subtree node so viewport-owned scaling can be reapplied without cumulative drift.
    /// </summary>
    public sealed class ViewportLayoutSnapshot {
        /// <summary>
        /// Initializes one snapshot for the supplied entity and any supported attached layout components.
        /// </summary>
        /// <param name="entity">Entity whose authored state should be preserved.</param>
        /// <param name="isRootEntity">True when the entity is the root of the scaled subtree.</param>
        public ViewportLayoutSnapshot(Entity entity, bool isRootEntity) {
            Entity = entity ?? throw new ArgumentNullException(nameof(entity));
            IsRootEntity = isRootEntity;
            LocalPosition = entity.LocalPosition;
            TrackedAnchorComponent = FindAnchorComponent(entity);
            if (TrackedAnchorComponent != null) {
                AnchorDistances = TrackedAnchorComponent.AnchorDistances;
            }
        }

        /// <summary>
        /// Gets the entity whose authored state is represented by the snapshot.
        /// </summary>
        public Entity Entity { get; }

        /// <summary>
        /// Gets a value indicating whether the snapshot belongs to the root entity of the scaled subtree.
        /// </summary>
        public bool IsRootEntity { get; }

        /// <summary>
        /// Applies one absolute viewport-driven scale to the captured entity and supported layout components.
        /// </summary>
        public void Apply(AnchorSpace anchorSpace, float2 canvasOrigin, int referenceWidth, int referenceHeight) {
            // Move the scaling math here from ReferenceCanvasFitSnapshot.
        }
    }
}
```

- [ ] **Step 4: Move scaling and explicit binding into `ViewportComponent`**

```csharp
public const byte ExplicitCameraBindingMode = 3;
public const byte NoScalingMode = 0;
public const byte ReferenceCanvasScalingMode = 1;

byte ScalingModeValue;
int ReferenceWidthValue;
int ReferenceHeightValue;
CameraComponent ExplicitBoundCameraComponentValue;
readonly List<ViewportLayoutSnapshot> LayoutSnapshotsValue;
bool PendingScaleApplyValue;

public byte ScalingMode {
    get { return ScalingModeValue; }
    set {
        if (ScalingModeValue != value) {
            ScalingModeValue = value;
            PendingScaleApplyValue = true;
            RaiseAnchorBoundsChanged();
        }
    }
}

public int ReferenceWidth {
    get { return ReferenceWidthValue; }
    set {
        if (value < 1) {
            throw new ArgumentOutOfRangeException(nameof(value), "Reference width must be at least one.");
        }

        ReferenceWidthValue = value;
        PendingScaleApplyValue = true;
    }
}

public CameraComponent BoundCameraComponent {
    get { return ExplicitBoundCameraComponentValue; }
    set {
        if (!ReferenceEquals(ExplicitBoundCameraComponentValue, value)) {
            ExplicitBoundCameraComponentValue = value;
            RefreshSubscriptions();
            RaiseAnchorBoundsChanged();
        }
    }
}
```

- [ ] **Step 5: Make `ReferenceCanvasFitComponent` a compatibility wrapper**

```csharp
public override void ComponentAdded(Entity entity) {
    base.ComponentAdded(entity);
    ViewportComponent viewportComponent = entity.GetComponent<ViewportComponent>();
    if (viewportComponent == null) {
        throw new InvalidOperationException("ReferenceCanvasFitComponent now requires ViewportComponent on the same entity.");
    }

    viewportComponent.ScalingMode = ViewportComponent.ReferenceCanvasScalingMode;
    viewportComponent.ReferenceWidth = ReferenceWidthValue;
    viewportComponent.ReferenceHeight = ReferenceHeightValue;
}
```

- [ ] **Step 6: Re-run the focused viewport tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~ReferenceCanvasFitComponentTests|FullyQualifiedName~ViewportAndAnchorLayoutTests" --no-restore`

Expected: PASS

- [ ] **Step 7: Commit the viewport contract slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.core/components/ViewportComponent.cs engine/helengine.core/components/ViewportLayoutSnapshot.cs engine/helengine.core/components/ReferenceCanvasFitComponent.cs engine/helengine.editor.tests/components/ReferenceCanvasFitComponentTests.cs engine/helengine.editor.tests/ViewportAndAnchorLayoutTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "feat: unify viewport scaling and camera binding"
```

### Task 2: Add Shared Menu Scene Routing

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.core\content\PlatformMenuSceneResolver.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\menu\MenuComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Write the failing return-to-menu routing test**

```csharp
[Fact]
public void RewriteGeneratedCore_WhenReturnToMenuLoadsMainMenu_RoutesThroughPlatformMenuSceneResolver() {
    string normalizedSource = RewriteReturnToMenuComponentSource();

    Assert.Contains("PlatformMenuSceneResolver", normalizedSource, StringComparison.Ordinal);
    Assert.Contains("ResolveMainMenuSceneId", normalizedSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the generated-core routing test**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests" --no-restore`

Expected: FAIL because generated code still references `MainMenuSceneId` directly.

- [ ] **Step 3: Add the shared platform menu resolver**

```csharp
namespace helengine {
    /// <summary>
    /// Resolves the platform-appropriate menu scene id for startup and return-to-menu flows.
    /// </summary>
    public static class PlatformMenuSceneResolver {
        /// <summary>
        /// Resolves the current platform menu scene id.
        /// </summary>
        /// <returns>Platform-appropriate menu scene id.</returns>
        public static string ResolveMainMenuSceneId() {
            if (Core.Instance == null || Core.Instance.InitializationOptions == null) {
                throw new InvalidOperationException("Core initialization options must exist before resolving the main menu scene.");
            }

            return Core.Instance.InitializationOptions.ScenePathResolver.ResolveScenePath("DemoDiscMainMenu");
        }
    }
}
```

- [ ] **Step 4: Route `MenuComponent` return-to-menu calls through the resolver seam**

```csharp
string resolvedMenuSceneId = PlatformMenuSceneResolver.ResolveMainMenuSceneId();
if (Core.Instance.SceneManager == null) {
    throw new InvalidOperationException("Core scene manager must be initialized before runtime menu scene loading can occur.");
}

Core.Instance.SceneManager.LoadScene(resolvedMenuSceneId, SceneLoadMode.Single);
```

- [ ] **Step 5: Update the generated-core rewrite expectations**

```csharp
Assert.Contains("PlatformMenuSceneResolver::ResolveMainMenuSceneId()", normalizedSource, StringComparison.Ordinal);
Assert.DoesNotContain("MainMenuSceneId, SceneLoadMode::Single", normalizedSource, StringComparison.Ordinal);
```

- [ ] **Step 6: Re-run the generated-core tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests" --no-restore`

Expected: PASS

- [ ] **Step 7: Commit the menu routing slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.core/content/PlatformMenuSceneResolver.cs engine/helengine.core/components/2d/menu/MenuComponent.cs engine/helengine.editor.tests/managers/project/EditorGeneratedCoreRegenerationServiceTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "feat: add platform-aware menu scene routing"
```

### Task 3: Rewrite Menu Generators For Desktop And DS

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\DemoMenuSceneAssetFactory.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\NintendoDsMenuSceneAssetFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\menu\DemoMenuSceneBuildService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\menu\EditorMenuSceneRegenerationServiceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`

- [ ] **Step 1: Write the failing generator tests**

```csharp
[Fact]
public void BuildSceneAsset_WhenGeneratingDesktopMenu_UsesViewportComponentWithoutReferenceCanvasFitComponent() {
    DemoMenuSceneAssetFactory factory = new DemoMenuSceneAssetFactory();
    SceneAsset sceneAsset = factory.BuildSceneAsset("DemoDiscMainMenu", "gameplay.MenuProvider, gameplay", CreateDefinition());

    SceneEntityAsset menuRoot = Assert.Single(sceneAsset.RootEntities, entity => entity.Name == "DemoDiscMenuRoot");
    Assert.Contains(menuRoot.Components, component => component.ComponentTypeId == "helengine.ViewportComponent, helengine.core");
    Assert.DoesNotContain(menuRoot.Components, component => component.ComponentTypeId == "helengine.ReferenceCanvasFitComponent, helengine.core");
}

[Fact]
public void BuildSceneAsset_WhenGeneratingNintendoDsMenu_ProducesTopAndBottomViewportRoots() {
    NintendoDsMenuSceneAssetFactory factory = new NintendoDsMenuSceneAssetFactory();
    SceneAsset sceneAsset = factory.BuildSceneAsset("DemoDiscMainMenuDs", "gameplay.MenuProvider, gameplay", CreateDefinition());

    Assert.Contains(sceneAsset.RootEntities, entity => entity.Name == "NintendoDsTopScreenCamera");
    Assert.Contains(sceneAsset.RootEntities, entity => entity.Name == "NintendoDsBottomScreenCamera");
    Assert.Contains(sceneAsset.RootEntities, entity => entity.Name == "NintendoDsTopScreenRoot");
    Assert.Contains(sceneAsset.RootEntities, entity => entity.Name == "NintendoDsBottomScreenRoot");
}
```

- [ ] **Step 2: Run the menu generator tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorMenuSceneRegenerationServiceTests|FullyQualifiedName~EditorWindowsBuildScenePackagerTests" --no-restore`

Expected: FAIL because desktop generator still emits `ReferenceCanvasFitComponent` and DS generator does not exist.

- [ ] **Step 3: Switch the desktop generator to viewport-owned scaling**

```csharp
ViewportComponent viewportComponent = new ViewportComponent {
    BindingMode = ViewportComponent.ScreenBindingMode,
    FixedSize = new int2(DemoMenuLayout.CanvasWidth, DemoMenuLayout.CanvasHeight),
    ScalingMode = ViewportComponent.ReferenceCanvasScalingMode,
    ReferenceWidth = DemoMenuLayout.CanvasWidth,
    ReferenceHeight = DemoMenuLayout.CanvasHeight
};

SceneComponentAssetRecord viewportRecord = AutomaticDescriptor.SerializeComponent(viewportComponent, 1, null);
return new SceneEntityAsset {
    Components = new[] { buildRecord, viewportRecord },
    Children = new[] { BuildGeneratedRootEntityAsset(definition) }
};
```

- [ ] **Step 4: Add the DS-specific menu factory**

```csharp
public SceneAsset BuildSceneAsset(string sceneId, string providerTypeName, MenuDefinition definition) {
    return new SceneAsset {
        Id = sceneId,
        AssetReferences = BuildAssetReferences(definition),
        RootEntities = new[] {
            BuildTopCameraEntityAsset(),
            BuildBottomCameraEntityAsset(),
            BuildTopScreenRootEntityAsset(providerTypeName, definition),
            BuildBottomScreenRootEntityAsset(providerTypeName, definition)
        }
    };
}
```

- [ ] **Step 5: Add DS scene selection to the build service**

```csharp
public SceneAsset BuildNintendoDsSceneAsset(string sceneId, string providerTypeName, MenuDefinition definition) {
    if (definition == null) {
        throw new ArgumentNullException(nameof(definition));
    }

    return NintendoDsSceneAssetFactory.BuildSceneAsset(sceneId, providerTypeName, definition);
}
```

- [ ] **Step 6: Re-run the generator tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorMenuSceneRegenerationServiceTests|FullyQualifiedName~EditorWindowsBuildScenePackagerTests" --no-restore`

Expected: PASS

- [ ] **Step 7: Commit the generator rewrite**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor/managers/menu/DemoMenuSceneAssetFactory.cs engine/helengine.editor/managers/menu/NintendoDsMenuSceneAssetFactory.cs engine/helengine.editor/managers/menu/DemoMenuSceneBuildService.cs engine/helengine.editor.tests/managers/menu/EditorMenuSceneRegenerationServiceTests.cs engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "feat: add DS-specific menu scene generator"
```

### Task 4: Route DS Startup To The DS-Owned Menu Scene

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorBuildQueueItemFactory.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsStartupSceneIds.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsGeneratedCoreStager.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformAssetBuilder.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsGeneratedCoreStagerTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Write the failing DS startup-scene tests**

```csharp
[Fact]
public void Create_WhenPlatformIsNintendoDs_OrdersDsMainMenuSceneFirst() {
    EditorBuildQueueItemFactory factory = CreateFactoryWithScenes("DemoDiscMainMenu", "DemoDiscMainMenuDs", "CubeTest");
    EditorBuildQueueItemDocument item = factory.Create(CreateNintendoDsPlatformConfig(), CreateSelectionModel(), "C:\\tmp\\out");

    Assert.Equal("DemoDiscMainMenuDs", Assert.First(item.SelectedSceneIds));
}

[Fact]
public void StageRuntimeStartupManifest_WhenNintendoDsBuild_RewritesStartupSceneToDsMenuScene() {
    string stagedSource = StageStartupManifestSource();

    Assert.Contains("DemoDiscMainMenuDs", stagedSource, StringComparison.Ordinal);
    Assert.Contains("cooked/scenes/DemoDiscMainMenuDs.hasset", stagedSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the DS startup-scene tests**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore`

Expected: FAIL because DS startup constants still point at `DemoDiscMainMenu`.

- [ ] **Step 3: Update DS startup-scene identifiers and editor build ordering**

```csharp
public const string DemoDiscMainMenuSceneId = "DemoDiscMainMenuDs";
public const string DemoDiscMainMenuCookedRelativePath = "cooked/scenes/DemoDiscMainMenuDs.hasset";
```

```csharp
const string NintendoDsStartupSceneId = "DemoDiscMainMenuDs";
```

- [ ] **Step 4: Rewrite staged manifest and staging validation to the DS-owned scene**

```csharp
return sourceCode
    .Replace(
        "static const char kRuntimeStartupSceneRelativePath[] = \"cooked/scenes/DemoDiscMainMenu.hasset\";",
        "static const char kRuntimeStartupSceneRelativePath[] = \"cooked/scenes/DemoDiscMainMenuDs.hasset\";")
    .Replace(
        "return \"cooked/scenes/DemoDiscMainMenu.hasset\";",
        "return \"cooked/scenes/DemoDiscMainMenuDs.hasset\";");
```

- [ ] **Step 5: Re-run the DS builder tests**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore`

Expected: PASS

- [ ] **Step 6: Commit the DS startup routing slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor/managers/project/EditorBuildQueueItemFactory.cs
rtk git -C C:\dev\helworks\helengine commit -m "feat: route DS builds to the DS menu scene"
rtk git -C C:\dev\helworks\helengine-ds add builder/NintendoDsStartupSceneIds.cs builder/NintendoDsGeneratedCoreStager.cs builder/NintendoDsPlatformAssetBuilder.cs builder.tests/NintendoDsGeneratedCoreStagerTests.cs builder.tests/NintendoDsPlatformAssetBuilderTests.cs
rtk git -C C:\dev\helworks\helengine-ds commit -m "feat: stage DS-owned menu startup scene"
```

### Task 5: Add DS Dual-Screen Viewport Mapping And Two-Camera Presentation

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\rendering\CameraViewportResolverTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsBootHost.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager3DSourceAuditTests.cs`

- [ ] **Step 1: Write the failing DS viewport mapping tests**

```csharp
[Fact]
public void ResolveViewport_WhenNintendoDsBottomScreenViewportIsNormalized_ReturnsBottomScreenPixels() {
    float4 resolvedViewport = CameraViewportResolver.ResolveViewport(new float4(0f, 1f, 1f, 1f), 256d, 384d);

    Assert.Equal(new float4(0f, 192f, 256f, 192f), resolvedViewport);
}

[Fact]
public void NintendoDsRenderManager3DSource_WhenReadingCameras_DoesNotAlwaysSelectTheFirstCamera() {
    string sourceCode = File.ReadAllText(SourcePath);

    Assert.DoesNotContain("ICamera* selectedCamera = (*cameras)[0];", sourceCode, StringComparison.Ordinal);
    Assert.Contains("ResolveViewportBoundCameraTargets", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the DS viewport tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CameraViewportResolverTests" --no-restore`

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests" --no-restore`

Expected: FAIL because shared tests do not cover stacked DS viewports and DS source still selects the first camera directly.

- [ ] **Step 3: Extend viewport resolution and DS camera selection**

```csharp
[Fact]
public void ResolveViewport_WhenViewportUsesBottomScreenNormalizedRegion_ReturnsStackedPixelBounds() {
    float4 resolvedViewport = CameraViewportResolver.ResolveViewport(new float4(0f, 1f, 1f, 1f), 256d, 384d);
    Assert.Equal(new float4(0f, 192f, 256f, 192f), resolvedViewport);
}
```

```cpp
std::vector<ICamera*> NintendoDsRenderManager3D::ResolveViewportBoundCameraTargets(List<ICamera*>* cameras) {
    std::vector<ICamera*> targets;
    for (int32_t index = 0; index < cameras->Count(); index++) {
        ICamera* camera = (*cameras)[index];
        if (camera == nullptr) {
            continue;
        }

        targets.push_back(camera);
    }

    return targets;
}
```

- [ ] **Step 4: Present top and bottom 2D/3D targets separately**

```cpp
for (ICamera* camera : resolvedTargets) {
    float4 viewport = camera->get_Viewport();
    if (viewport.Y >= 1.0f) {
        renderManager2D->DrawCameraToBottomScreen(camera);
    } else {
        renderManager2D->DrawCameraToTopScreen(camera);
    }
}
```

- [ ] **Step 5: Re-run the viewport mapping and DS source-audit tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CameraViewportResolverTests" --no-restore`

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests" --no-restore`

Expected: PASS

- [ ] **Step 6: Commit the dual-screen DS renderer slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/rendering/CameraViewportResolverTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "test: cover stacked DS viewport resolution"
rtk git -C C:\dev\helworks\helengine-ds add src/platform/ds/NintendoDsBootHost.cpp src/platform/ds/NintendoDsRenderManager3D.cpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager3DSourceAuditTests.cs
rtk git -C C:\dev\helworks\helengine-ds commit -m "feat: render DS dual-screen viewport cameras"
```

### Task 6: End-To-End Verification

**Files:**
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Scenes\DemoDiscMainMenu.helen` only if the generator rewrite explicitly requires regeneration
- Create: `C:\tmp\helengine-ds-city-cube-project\city\assets\Scenes\DemoDiscMainMenuDs.helen` if the DS scene is materialized in the temp project during verification

- [ ] **Step 1: Regenerate the menu scenes through the editor-owned path**

Run:

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorMenuSceneRegenerationServiceTests" --no-restore
```

Expected: PASS and confirms generator-owned scene emission is valid before export.

- [ ] **Step 2: Run the complete shared-engine regression slice**

Run:

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~ReferenceCanvasFitComponentTests|FullyQualifiedName~ViewportAndAnchorLayoutTests|FullyQualifiedName~CameraViewportResolverTests|FullyQualifiedName~EditorMenuSceneRegenerationServiceTests|FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests|FullyQualifiedName~EditorWindowsBuildScenePackagerTests" --no-restore
```

Expected: PASS

- [ ] **Step 3: Run the complete DS builder regression slice**

Run:

```bash
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests|FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests" --no-restore -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\ds-dual-screen-menu\'
```

Expected: PASS

- [ ] **Step 4: Build the DS project end to end**

Run:

```bash
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-dual-screen-menu\bin\' -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: `Build completed for platform 'ds': C:\tmp\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 5: Launch `melonDS` and verify the dual-screen menu behavior**

Run:

```bash
& "C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe" "C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds"
```

Expected:

- top screen shows logo and title only
- bottom screen shows the interactive menu list
- return-to-menu resolves back to the DS menu scene

- [ ] **Step 6: Commit final verification artifacts only if project-authored scene files changed**

```bash
rtk git -C C:\tmp\helengine-ds-city-cube-project status --short
```

Expected: inspect whether regenerated scene files should be preserved or ignored before any client-project commit.

## Self-Review

- Spec coverage:
  - shared `ViewportComponent` ownership is covered by Task 1
  - platform-aware menu routing is covered by Task 2
  - separate DS generator is covered by Task 3
  - DS startup-scene selection is covered by Task 4
  - DS stacked-screen viewport mapping and dual-camera rendering are covered by Task 5
  - integration verification in `melonDS` is covered by Task 6
- Placeholder scan:
  - no `TODO`, `TBD`, or “implement later” markers remain
  - each task has explicit commands and code seams
- Type consistency:
  - plan consistently uses `ScalingMode`, `ReferenceWidth`, `ReferenceHeight`, `BoundCameraComponent`, and `ExplicitCameraBindingMode` on `ViewportComponent`
  - DS menu scene id consistently uses `DemoDiscMainMenuDs`

