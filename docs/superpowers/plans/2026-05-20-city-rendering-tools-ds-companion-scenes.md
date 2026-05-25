# City Rendering.Tools DS Companion Scenes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Generate DS companion scenes for the `city` rendering showcase scenes, with SceneMap remapping from default ids to DS ids and a default bottom-screen debug/back overlay.

**Architecture:** Keep the feature local to `city/assets/codebase/rendering.tools` by extending generated-scene definitions, centralizing DS dual-screen scaffold creation in one helper, and emitting both normal and DS companion scenes from the existing generator flow. Keep menu providers unchanged and rely on SceneMap remaps from default scene ids to DS companion scene ids.

**Tech Stack:** C#, city rendering tools, helengine scene serialization, xUnit authoring/runtime tests, `dotnet test`, `rtk`, melonDS.

---

### Task 1: Extend Generated Scene Definitions for DS Companion Output

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneDefinition.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedDsSceneDefinition.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`

- [ ] **Step 1: Write the failing authoring test for DS companion scene metadata**

Add one source-audit style test to `CityRenderingSceneAuthoringTests.cs`:

```csharp
        [Fact]
        public void ReadCityGeneratedAuthoringSceneDefinitionSource_DeclaresDsCompanionSceneMetadata() {
            string source = ReadCitySource("rendering.tools", "GeneratedAuthoringSceneDefinition.cs");

            Assert.Contains("public GeneratedDsSceneDefinition NintendoDsScene { get; set; }", source, StringComparison.Ordinal);
        }
```

Add one test for the new DS definition type:

```csharp
        [Fact]
        public void ReadCityGeneratedDsSceneDefinitionSource_DeclaresBottomOverlayCustomizationContract() {
            string source = ReadCitySource("rendering.tools", "GeneratedDsSceneDefinition.cs");

            Assert.Contains("public string SceneId { get; set; }", source, StringComparison.Ordinal);
            Assert.Contains("public bool UseDefaultBottomOverlay { get; set; }", source, StringComparison.Ordinal);
            Assert.Contains("public Entity[] BottomScreenRootEntities { get; set; }", source, StringComparison.Ordinal);
        }
```

- [ ] **Step 2: Run the focused city authoring test slice and verify it fails**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: FAIL with missing `NintendoDsScene` and missing `GeneratedDsSceneDefinition.cs` assertions.

- [ ] **Step 3: Add the DS companion scene definition types**

Create `GeneratedDsSceneDefinition.cs`:

```csharp
namespace city.rendering.tools {
    /// <summary>
    /// Stores one generated Nintendo DS companion-scene definition emitted alongside a default generated rendering scene.
    /// </summary>
    public sealed class GeneratedDsSceneDefinition {
        /// <summary>
        /// Gets or sets the stable DS companion scene id written to disk.
        /// </summary>
        public string SceneId { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the standard DS bottom overlay should be emitted automatically.
        /// </summary>
        public bool UseDefaultBottomOverlay { get; set; }

        /// <summary>
        /// Gets or sets optional custom bottom-screen root entities supplied by one generator when it opts out of the default overlay.
        /// </summary>
        public Entity[] BottomScreenRootEntities { get; set; }
    }
}
```

Update `GeneratedAuthoringSceneDefinition.cs`:

```csharp
        /// <summary>
        /// Gets or sets the optional Nintendo DS companion-scene definition emitted alongside the default generated scene.
        /// </summary>
        public GeneratedDsSceneDefinition NintendoDsScene { get; set; }
```

- [ ] **Step 4: Run the focused authoring test slice and verify it passes**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: PASS for the new DS-definition tests.

- [ ] **Step 5: Commit**

```bash
git add C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneDefinition.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedDsSceneDefinition.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs
git commit -m "Add DS companion scene metadata contract"
```

### Task 2: Add a Shared DS Scene Scaffold Builder

**Files:**
- Create: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`

- [ ] **Step 1: Write the failing source-audit test for the DS scaffold helper**

Add:

```csharp
        [Fact]
        public void ReadCityNintendoDsRenderingSceneScaffoldFactorySource_BuildsDualScreenDebugAndBackOverlay() {
            string source = ReadCitySource("rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs");

            Assert.Contains("DemoDiscTopScreenCamera", source, StringComparison.Ordinal);
            Assert.Contains("DemoDiscBottomScreenCamera", source, StringComparison.Ordinal);
            Assert.Contains("new DebugComponent()", source, StringComparison.Ordinal);
            Assert.Contains("new DemoDiscReturnToMenuComponent()", source, StringComparison.Ordinal);
        }
```

- [ ] **Step 2: Run the focused city authoring test slice and verify it fails**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: FAIL because `NintendoDsRenderingSceneScaffoldFactory.cs` does not exist yet.

- [ ] **Step 3: Create the shared DS scaffold helper**

Create `NintendoDsRenderingSceneScaffoldFactory.cs` with a focused contract:

```csharp
namespace city.rendering.tools {
    /// <summary>
    /// Builds the shared Nintendo DS dual-screen scaffold used by generated city rendering showcase companion scenes.
    /// </summary>
    public sealed class NintendoDsRenderingSceneScaffoldFactory {
        /// <summary>
        /// Builds one DS companion-scene root set from top-screen scene content and optional custom bottom-screen content.
        /// </summary>
        /// <param name="topScreenRoots">Scene roots that should render on the top screen.</param>
        /// <param name="useDefaultBottomOverlay">True to emit the standard bottom debug/back overlay.</param>
        /// <param name="bottomScreenRoots">Optional custom bottom-screen roots supplied by the generator.</param>
        /// <returns>Dual-screen DS scene roots.</returns>
        public Entity[] CreateSceneRoots(Entity[] topScreenRoots, bool useDefaultBottomOverlay, Entity[] bottomScreenRoots) {
            throw new NotImplementedException();
        }
    }
```

Then implement:

- one top camera root named `DemoDiscTopScreenCamera`
- one bottom camera root named `DemoDiscBottomScreenCamera`
- full-screen DS viewports for top and bottom
- default bottom overlay subtree that contains:
  - `DebugComponent`
  - back button entity
  - `DemoDiscReturnToMenuComponent`

Keep the implementation in this file only; do not spread DS scaffold creation across every scene factory.

- [ ] **Step 4: Ensure `GeneratedAuthoringSceneWriteService` can serialize the scaffold’s existing components**

Confirm or add registrations needed by the scaffold:

```csharp
            persistenceRegistry.Register(new DebugComponentPersistenceDescriptor());
            persistenceRegistry.Register(new CameraComponentPersistenceDescriptor());
            persistenceRegistry.Register(new TextComponentPersistenceDescriptor());
            persistenceRegistry.Register(new RoundedRectComponentPersistenceDescriptor());
```

Only add new registrations if the DS scaffold introduces a component not already registered.

- [ ] **Step 5: Run the focused authoring test slice and verify it passes**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: PASS for the new scaffold source-audit.

- [ ] **Step 6: Commit**

```bash
git add C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs
git commit -m "Add shared DS rendering scene scaffold"
```

### Task 3: Emit DS Companion Scene Definitions from RenderingSceneGenerator

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerator.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\CubeTestSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\ScaledCubeSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\ColoredCubeGridSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\TexturedCubeGridSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\AxisTestSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\AxisTest2SceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\DirectionalShadowPlazaSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\SpotlightStreetSliceSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\SceneMemoryProbeSceneFactory.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`

- [ ] **Step 1: Write failing authoring tests for DS companion ids**

Add source-audits that verify the generator now knows DS scene ids:

```csharp
        [Fact]
        public void ReadCityRenderingSceneGeneratorSource_DeclaresDsCompanionSceneIds() {
            string source = ReadCitySource("rendering.tools", "RenderingSceneGenerator.cs");

            Assert.Contains("CubeTestNintendoDsSceneId", source, StringComparison.Ordinal);
            Assert.Contains("AxisTestNintendoDsSceneId", source, StringComparison.Ordinal);
            Assert.Contains("SceneMemoryProbeNintendoDsSceneId", source, StringComparison.Ordinal);
        }
```

Add one factory-level source audit:

```csharp
        [Fact]
        public void ReadCityCubeTestSceneFactorySource_AttachesNintendoDsSceneDefinition() {
            string source = ReadCitySource("rendering.tools", "CubeTestSceneFactory.cs");

            Assert.Contains("NintendoDsScene = new GeneratedDsSceneDefinition", source, StringComparison.Ordinal);
            Assert.Contains("UseDefaultBottomOverlay = true", source, StringComparison.Ordinal);
        }
```

- [ ] **Step 2: Run the focused authoring test slice and verify it fails**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: FAIL because the DS ids and `NintendoDsScene` assignments do not exist yet.

- [ ] **Step 3: Add stable DS companion ids to `RenderingSceneGenerator`**

For each existing scene id constant, add a DS companion id constant, for example:

```csharp
        public const string CubeTestNintendoDsSceneId = "scenes/rendering/ds/cube_test_ds.helen";
        public const string AxisTestNintendoDsSceneId = "scenes/rendering/ds/axis_test_ds.helen";
```

Apply the same pattern consistently across all generated rendering scenes.

- [ ] **Step 4: Update each scene factory to describe its DS companion scene**

For each factory’s returned `GeneratedAuthoringSceneDefinition`, add:

```csharp
                NintendoDsScene = new GeneratedDsSceneDefinition {
                    SceneId = RenderingSceneGenerator.CubeTestNintendoDsSceneId,
                    UseDefaultBottomOverlay = true,
                    BottomScreenRootEntities = Array.Empty<Entity>()
                },
```

For now, all existing rendering showcase factories should use the default bottom overlay unless one already has a known reason not to.

- [ ] **Step 5: Run the focused authoring test slice and verify it passes**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: PASS for the new DS companion-id and factory metadata tests.

- [ ] **Step 6: Commit**

```bash
git add C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerator.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\CubeTestSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\ScaledCubeSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\ColoredCubeGridSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\TexturedCubeGridSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\AxisTestSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\AxisTest2SceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\DirectionalShadowPlazaSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\SpotlightStreetSliceSceneFactory.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\SceneMemoryProbeSceneFactory.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs
git commit -m "Describe DS companion scenes for rendering showcases"
```

### Task 4: Write Both Normal and DS Scenes from the Generator Flow

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerator.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs`

- [ ] **Step 1: Write failing source-audits for dual output**

Add:

```csharp
        [Fact]
        public void ReadCityGeneratedAuthoringSceneWriteServiceSource_WritesNintendoDsCompanionScenes() {
            string source = ReadCitySource("rendering.tools", "GeneratedAuthoringSceneWriteService.cs");

            Assert.Contains("sceneDefinition.NintendoDsScene", source, StringComparison.Ordinal);
            Assert.Contains("NintendoDsRenderingSceneScaffoldFactory", source, StringComparison.Ordinal);
        }
```

And:

```csharp
        [Fact]
        public void ReadCityRenderingSceneGeneratorSource_WritesBothDefaultAndNintendoDsSceneOutputs() {
            string source = ReadCitySource("rendering.tools", "RenderingSceneGenerator.cs");

            Assert.Contains("AuthoringSceneWriteService.WriteScene(projectRootPath, cubeTestSceneDefinition);", source, StringComparison.Ordinal);
            Assert.Contains("sceneDefinition.NintendoDsScene", ReadCitySource("rendering.tools", "GeneratedAuthoringSceneWriteService.cs"), StringComparison.Ordinal);
        }
```

- [ ] **Step 2: Run the focused authoring test slice and verify it fails**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: FAIL because the write service does not yet emit DS companion scenes.

- [ ] **Step 3: Implement DS companion-scene writing in `GeneratedAuthoringSceneWriteService`**

Keep `WriteScene(...)` as the public entry point, but extend it to:

- persist the default scene exactly as today
- detect `sceneDefinition.NintendoDsScene`
- build DS scaffold roots from the default scene roots plus DS metadata
- persist the DS companion scene to its own scene id/path

The write flow should remain deterministic and dispose any generated roots it creates.

- [ ] **Step 4: Keep `RenderingSceneGenerator` calling the same write entry point**

Do not add a second public write call in `RenderingSceneGenerator`. The DS companion write should remain encapsulated inside the write service so the generator remains simple.

- [ ] **Step 5: Run the focused authoring test slice and verify it passes**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests" -v minimal
```

Expected: PASS for the write-service source-audits.

- [ ] **Step 6: Commit**

```bash
git add C:\dev\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerator.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\CityRenderingSceneAuthoringTests.cs
git commit -m "Write DS companion rendering scenes"
```

### Task 5: Emit SceneMap Remaps for Generated DS Companion Scenes

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerator.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
- Possibly modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscSceneGenerator.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`

- [ ] **Step 1: Write the failing SceneMap test for one generated rendering scene remap**

Add a test that verifies the generated boot/scene-map path contains a remap for one rendering scene:

```csharp
        [Fact]
        public void SceneMapComponent_WhenCityDsCompanionScenesExist_resolvesDefaultRenderingSceneIdToNintendoDsSceneId() {
            SceneMapComponent sceneMapComponent = new SceneMapComponent();
            sceneMapComponent.Mappings.Add("scenes/rendering/cube_test.helen", "scenes/rendering/ds/cube_test_ds.helen");

            string resolvedSceneId = SceneMapComponent.ResolveSceneId("scenes/rendering/cube_test.helen");

            Assert.Equal("scenes/rendering/ds/cube_test_ds.helen", resolvedSceneId);
        }
```

If there is already a generation/seeding test seam for DS SceneMap content, extend that instead of inventing a disconnected test.

- [ ] **Step 2: Run the focused SceneMap test slice and verify it fails at the generation seam**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" -v minimal
```

Expected: FAIL where the DS generation path does not yet seed the generated rendering scene remaps.

- [ ] **Step 3: Update the city generation flow to emit default-id to DS-id remaps**

Add deterministic remap entries for every rendering-tools generated scene, for example:

```csharp
cubeTestDefaultSceneId -> cubeTestNintendoDsSceneId
axisTestDefaultSceneId -> axisTestNintendoDsSceneId
sceneMemoryProbeDefaultSceneId -> sceneMemoryProbeNintendoDsSceneId
```

Hook this into the same city DS boot/menu generation seam that already emits `DemoDiscMainMenu -> DemoDiscMainMenuDs`.

- [ ] **Step 4: Run the focused SceneMap test slice and verify it passes**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests" -v minimal
```

Expected: PASS for the new generated-scene remap behavior.

- [ ] **Step 5: Commit**

```bash
git add C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerator.cs C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscSceneGenerator.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs
git commit -m "Map default rendering scenes to DS companion scenes"
```

### Task 6: End-to-End DS Generation Verification

**Files:**
- Verify: `C:\dev\helprojs\city\assets\Scenes`
- Verify: `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

- [ ] **Step 1: Run the focused city authoring and SceneMap test slices**

Run:

```powershell
rtk proxy dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityRenderingSceneAuthoringTests|FullyQualifiedName~SceneMapServiceTests" -v minimal
```

Expected: PASS with zero failures.

- [ ] **Step 2: Regenerate the city scenes and build the DS ROM**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-editor-font\bin\' -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: `Build completed for platform 'ds': C:\tmp\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 3: Launch melonDS with the rebuilt ROM**

Run:

```powershell
$processes = Get-Process melonDS -ErrorAction SilentlyContinue; if ($processes) { $processes | Stop-Process -Force }; Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'
```

Expected: melonDS opens the rebuilt ROM.

- [ ] **Step 4: Manually verify the DS companion-scene behavior**

Manual checklist:

```text
1. Select one generated rendering scene from the DS menu using the normal menu entry.
2. Confirm the selected default scene id loads a DS companion scene through SceneMap.
3. Confirm the top screen shows the scene’s 3D content.
4. Confirm the bottom screen shows DebugComponent plus a back button by default.
5. Press the back button and confirm it returns to the DS menu.
```

- [ ] **Step 5: Commit the final verification checkpoint**

```bash
git commit --allow-empty -m "Verify DS companion scene generation flow"
```
