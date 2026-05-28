using System.Text;

namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the shared city project files that drive Nintendo DS boot remapping and DS-only scene generation.
/// </summary>
public class CityNintendoDsSceneSourceAuditTests {
    /// <summary>
    /// Absolute path of the city project used by the local DS integration workflow.
    /// </summary>
    const string CityProjectRootPath = @"C:\dev\helprojs\city";

    /// <summary>
    /// Verifies the city source tree still contains the menu generator and DS remap codepaths required by the DS boot flow.
    /// </summary>
    [Fact]
    public void Sources_whenDsBootFlowIsSupported_includeSceneMapResolutionAndDsCompanionGeneration() {
        string menuComponentSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "MenuComponent.cs"));
        string returnToMenuSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "DemoDiscReturnToMenuComponent.cs"));
        string dsReturnOverlaySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "NintendoDsReturnOverlayComponent.cs"));
        string platformInfoTextComponentSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "PlatformInfoTextComponent.cs"));
        string demoDiscSceneCatalogSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "DemoDiscSceneCatalog.cs"));
        string generatedAuthoringWriterSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "GeneratedAuthoringSceneWriteService.cs"));
        string buildConfigSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "user_settings", "build_config.json"));

        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscMainMenuSceneFactory.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscSceneGenerator.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "RegenerateDemoDiscMainMenuCommand.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "GeneratePhysicsNintendoDsScenesCommand.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "PhysicsNintendoDsSceneGenerator.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs")));

        Assert.Contains("SceneMapComponent.ResolveSceneId(sceneId)", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("inputSystem.WasGamepadButtonPressed(0, InputGamepadButton.DPadUp)", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("inputSystem.WasGamepadButtonPressed(0, InputGamepadButton.DPadDown)", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("StandardPlatformAction.Accept", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("StandardPlatformAction.Return", menuComponentSource, StringComparison.Ordinal);
        Assert.DoesNotContain("inputSystem.WasGamepadButtonPressed(0, InputGamepadButton.South)", menuComponentSource, StringComparison.Ordinal);
        Assert.DoesNotContain("inputSystem.WasGamepadButtonPressed(0, InputGamepadButton.East)", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("ResolvePointerXInMenuSpace", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("ResolvePointerYInMenuSpace", menuComponentSource, StringComparison.Ordinal);
        Assert.Contains("SceneMapComponent.ResolveSceneId(MainMenuSceneId)", returnToMenuSource, StringComparison.Ordinal);
        Assert.Contains("StandardPlatformAction.Return", returnToMenuSource, StringComparison.Ordinal);
        Assert.Contains("SceneMapComponent.ResolveSceneId(MainMenuSceneId)", dsReturnOverlaySource, StringComparison.Ordinal);
        Assert.Contains("StandardPlatformAction.Return", dsReturnOverlaySource, StringComparison.Ordinal);
        Assert.DoesNotContain("inputSystem.WasGamepadButtonPressed(0, InputGamepadButton.East)", dsReturnOverlaySource, StringComparison.Ordinal);
        Assert.Contains("Core.Instance.PlatformInfo", platformInfoTextComponentSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PlayStation 2", platformInfoTextComponentSource, StringComparison.Ordinal);
        Assert.DoesNotContain("cube_test", platformInfoTextComponentSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_stack_boxes", demoDiscSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_sphere_stack", demoDiscSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_mixed_stack", demoDiscSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("sceneDefinition.NintendoDsScene != null", generatedAuthoringWriterSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderingSceneScaffoldFactoryValue.CreateSceneRoots", generatedAuthoringWriterSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_dynamic_stack_boxes_ds\"", buildConfigSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_dynamic_sphere_stack_ds\"", buildConfigSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_dynamic_mixed_stack_ds\"", buildConfigSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the city Nintendo DS main-menu logo uses a fixed top-screen width and derives height only from the authored aspect ratio.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringDsMainMenuLogo_useFixedDsWidthWithoutBoundingBoxFitScaling() {
        string mainMenuFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscMainMenuSceneFactory.cs"));

        Assert.Contains("const int NintendoDsLogoWidth = 180;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("int displayWidth = NintendoDsLogoWidth;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("NintendoDsLogoMaxWidth", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("NintendoDsLogoMaxHeight", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("double widthScale = (double)NintendoDsLogoMaxWidth / overlayImage.Width;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("double heightScale = (double)NintendoDsLogoMaxHeight / overlayImage.Height;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("double scale = Math.Min(widthScale, heightScale);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("return NintendoDsLogoWidth;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("double aspectRatio = (double)overlayImage.Height / overlayImage.Width;", mainMenuFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the city Nintendo DS main-menu platform info uses a larger single-row left/right layout while preserving the shared runtime component for desktop scenes.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringDsMainMenuPlatformInfo_useHorizontalSplitLayoutWithLargerFontScale() {
        string mainMenuFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscMainMenuSceneFactory.cs"));
        string platformInfoComponentSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "PlatformInfoTextComponent.cs"));

        Assert.Contains("CreateTextEntity(entity, \"DemoDiscPlatformInfoNameText\", new float3(0f, 0f, 0f), string.Empty, definition.BodyFontPath, definition.TextColor, new int2(1, 1), 42, null, 0.84f, false);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateTextEntity(entity, \"DemoDiscPlatformInfoVersionText\", new float3(240f, 0f, 0f), string.Empty, definition.BodyFontPath, definition.MutedTextColor, new int2(1, 1), 42, null, 0.84f, false);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("float3 PlatformNameBaseLocalPosition;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("float3 PlatformVersionBaseLocalPosition;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("bool useHorizontalRowLayout = Math.Abs(PlatformNameBaseLocalPosition.Y - PlatformVersionBaseLocalPosition.Y) < 0.01f;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("ApplyHorizontalText(PlatformNameTextEntity, PlatformNameTextComponent, Core.Instance.PlatformInfo.Name, PlatformNameBaseLocalPosition.X, PlatformNameBaseLocalPosition.Y, TextAlignment.Left);", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("ApplyHorizontalText(PlatformVersionTextEntity, PlatformVersionTextComponent, Core.Instance.PlatformInfo.Version, PlatformVersionBaseLocalPosition.X, PlatformVersionBaseLocalPosition.Y, TextAlignment.Right);", platformInfoComponentSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the authored cube-test scene starts stationary and only rotates when the player drives orbit input.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringCubeTest_disableDefaultAutoYawRotation() {
        string cubeTestSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "CubeTestSceneFactory.cs"));

        Assert.Contains("AutoYawSpeedRadians = 0f", cubeTestSceneFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("AutoYawSpeedRadians = 0.1f", cubeTestSceneFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the menu-visible rendering demo scenes expose platform-specific instruction overlays and a truthful gamepad light-toggle binding.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringRenderingDemoInstructions_includePlatformSpecificIconOverlayAndGamepadLightToggle() {
        string overlayFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "DemoSceneInstructionOverlayFactory.cs"));
        string platformSelectorSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering", "DemoScenePlatformInstructionIconSetComponent.cs"));
        string lightToggleSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering", "DemoDiscLightToggleComponent.cs"));
        string cubeTestSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "CubeTestSceneFactory.cs"));
        string scaledCubeSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "ScaledCubeSceneFactory.cs"));
        string coloredCubeGridSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "ColoredCubeGridSceneFactory.cs"));
        string texturedCubeGridSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "TexturedCubeGridSceneFactory.cs"));
        string axisTestSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "AxisTestSceneFactory.cs"));
        string axisTest2SceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "AxisTest2SceneFactory.cs"));
        string directionalShadowPlazaSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "DirectionalShadowPlazaSceneFactory.cs"));

        Assert.Contains("Images/Instructions/Controls/xbox360_dpad.png", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("Images/Instructions/Controls/xbox360_rb.png", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("Images/Instructions/Controls/ps2_dpad.png", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("Images/Instructions/Controls/ps2_r1.png", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("Images/Instructions/Controls/switch_dpad.png", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("Images/Instructions/Controls/switch_r.png", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("const ushort DesktopOverlayLayerMask = EditorLayerMasks.SceneObjects;", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("const ushort NintendoDsOverlayLayerMask = 0b0000000000000001;", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("BindingMode = ViewportComponent.ScreenBindingMode", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("Entity viewportRootEntity = Core.Instance.EntityFactory.Create(\"DemoSceneInstructionViewport\");", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("viewportRootEntity.LayerMask = DesktopOverlayLayerMask;", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("panelEntity.LayerMask = DesktopOverlayLayerMask;", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("panelEntity.LayerMask = NintendoDsOverlayLayerMask;", overlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("ContainsNormalizedPlatformToken(normalizedPlatformName, \"windows\")", platformSelectorSource, StringComparison.Ordinal);
        Assert.Contains("ContainsNormalizedPlatformToken(normalizedPlatformName, \"gamecube\")", platformSelectorSource, StringComparison.Ordinal);
        Assert.Contains("ContainsNormalizedPlatformToken(normalizedPlatformName, \"ps2\")", platformSelectorSource, StringComparison.Ordinal);
        Assert.Contains("ContainsNormalizedPlatformToken(normalizedPlatformName, \"psp\")", platformSelectorSource, StringComparison.Ordinal);
        Assert.Contains("ContainsNormalizedPlatformToken(normalizedPlatformName, \"3ds\")", platformSelectorSource, StringComparison.Ordinal);
        Assert.Contains("normalizedPlatformName == \"ds\"", platformSelectorSource, StringComparison.Ordinal);
        Assert.Contains("InputGamepadButton.RightShoulder", lightToggleSource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", cubeTestSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", cubeTestSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", scaledCubeSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", scaledCubeSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", coloredCubeGridSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", coloredCubeGridSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", texturedCubeGridSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", texturedCubeGridSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", axisTestSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", axisTestSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", axisTest2SceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", axisTest2SceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateDesktopInstructionOverlayRoot(instructionFont);", directionalShadowPlazaSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsBottomInstructionRoots(instructionFont)", directionalShadowPlazaSceneFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the three menu-visible physics showcase scenes expose orbit controls plus desktop and DS instruction overlays, and that light toggling is implemented instead of stubbed.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringPlayablePhysicsShowcases_includeOrbitCameraInstructionsAndFunctionalLightToggle() {
        string physicsSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneFactory.cs"));
        string dsPhysicsSceneGeneratorSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsNintendoDsSceneGenerator.cs"));
        string lightToggleSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering", "DemoDiscLightToggleComponent.cs"));

        Assert.Contains("SceneAsset CreateDynamicStackBoxesScene()", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("SceneAsset CreateDynamicSphereStackScene()", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("SceneAsset CreateDynamicMixedStackScene()", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("new city.rendering.DemoDiscOrbitCameraComponent", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("AutoYawSpeedRadians = 0f", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("new city.rendering.DemoDiscLightToggleComponent()", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("new FPSComponent", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("new city.menu.DemoDiscReturnToMenuComponent()", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreatePhysicsShowcaseDesktopInstructionOverlayRoot()", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreatePhysicsShowcaseSceneAsset(", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreatePhysicsShowcaseNintendoDsBottomInstructionRoots()", dsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("CreatePlayablePhysicsShowcaseSceneDefinition(", dsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_dynamic_stack_boxes\"", dsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_dynamic_sphere_stack\"", dsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_dynamic_mixed_stack\"", dsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("LightStates.Add(new DemoDiscDirectionalLightToggleState", lightToggleSource, StringComparison.Ordinal);
        Assert.Contains("if (entity.Components[componentIndex] is DirectionalLightComponent directionalLightComponent)", lightToggleSource, StringComparison.Ordinal);
        Assert.Contains("directionalLightComponent.Intensity = 0f;", lightToggleSource, StringComparison.Ordinal);
        Assert.Contains("directionalLightComponent.ShadowsEnabled = false;", lightToggleSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the committed city scene assets still expose the DS menu scene and DS boot-scene mappings.
    /// </summary>
    [Fact]
    public void Assets_whenDsBootFlowIsSupported_includeDsMenuSceneAndDsPlayableSceneMappings() {
        string dsMenuScenePath = Path.Combine(CityProjectRootPath, "assets", "scenes", "DemoDiscMainMenuDs.helen");
        string generatedBootScenePath = Path.Combine(CityProjectRootPath, "assets", "scenes", "GeneratedBootScene.helen");

        Assert.True(File.Exists(dsMenuScenePath));
        Assert.True(File.Exists(generatedBootScenePath));

        string dsMenuSceneSource = Encoding.ASCII.GetString(File.ReadAllBytes(dsMenuScenePath));
        string generatedBootSceneSource = Encoding.ASCII.GetString(File.ReadAllBytes(generatedBootScenePath));

        Assert.Contains("DemoDiscTopScreenRoot", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("DemoDiscBottomScreenCamera", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("DemoDiscMenuRoot", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.DoesNotContain("DemoDiscBottomScreenRoot", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("DemoDiscMainMenuDs", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.Contains("cube_test_ds", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.Contains("axis_test_ds", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.Contains("colored_cube_grid_ds", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.Contains("textured_cube_grid_ds", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "scenes", "physics", "test_scene_dynamic_stack_boxes_ds.helen")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "scenes", "physics", "test_scene_dynamic_sphere_stack_ds.helen")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "scenes", "physics", "test_scene_dynamic_mixed_stack_ds.helen")));
        Assert.Contains("test_scene_dynamic_stack_boxes_ds", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_sphere_stack_ds", generatedBootSceneSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_mixed_stack_ds", generatedBootSceneSource, StringComparison.Ordinal);

        string stackedBoxesDsSceneSource = Encoding.ASCII.GetString(File.ReadAllBytes(Path.Combine(CityProjectRootPath, "assets", "scenes", "physics", "test_scene_dynamic_stack_boxes_ds.helen")));
        Assert.Contains("Materials/physics/PhysicsDemoBlue.hasset", stackedBoxesDsSceneSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the shared physics demo materials are authored through the per-platform material settings flow instead of legacy raw material payloads.
    /// </summary>
    [Fact]
    public void Assets_whenPhysicsDsScenesAreSupported_includeModernPerPlatformPhysicsMaterialSettings() {
        AssertPhysicsMaterialIsStoredWithPerPlatformSettings("PhysicsDemoNeutral");
        AssertPhysicsMaterialIsStoredWithPerPlatformSettings("PhysicsDemoBlue");
        AssertPhysicsMaterialIsStoredWithPerPlatformSettings("PhysicsDemoGreen");
        AssertPhysicsMaterialIsStoredWithPerPlatformSettings("PhysicsDemoMagenta");
        AssertPhysicsMaterialIsStoredWithPerPlatformSettings("PhysicsDemoYellow");
    }

    /// <summary>
    /// Verifies one shared physics demo material uses the common plus per-platform sidecar format required by DS cooking.
    /// </summary>
    /// <param name="materialName">Stable physics demo material asset name.</param>
    static void AssertPhysicsMaterialIsStoredWithPerPlatformSettings(string materialName) {
        string baseMaterialPath = Path.Combine(CityProjectRootPath, "assets", "materials", "physics", materialName + ".hasset");

        Assert.True(File.Exists(baseMaterialPath));

        string baseMaterialSource = Encoding.ASCII.GetString(File.ReadAllBytes(baseMaterialPath));
        Assert.Contains("helengine.material", baseMaterialSource, StringComparison.Ordinal);
        Assert.DoesNotContain("ps2-simple-lit-textured", baseMaterialSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the packager source lowercases cooked file-system material reference paths so DS runtime lookup matches NitroFS staging paths.
    /// </summary>
    [Fact]
    public void Sources_whenPackagingFileSystemMaterials_lowercaseCookedMaterialReferencePaths() {
        string packagingTransformSource = File.ReadAllText(Path.Combine(
            @"C:\dev\helworks\helengine",
            "engine",
            "helengine.editor",
            "managers",
            "project",
            "SceneComponentPackagingTransformService.cs"));
        string windowsPackagerSource = File.ReadAllText(Path.Combine(
            @"C:\dev\helworks\helengine",
            "engine",
            "helengine.editor",
            "managers",
            "project",
            "EditorWindowsBuildScenePackager.cs"));

        Assert.Contains("NormalizeRelativePath(Path.Combine(\"cooked\", normalizedRelativePath)).ToLowerInvariant()", packagingTransformSource, StringComparison.Ordinal);
        Assert.Contains("NormalizeRelativePath(Path.Combine(\"cooked\", normalizedRelativePath)).ToLowerInvariant()", windowsPackagerSource, StringComparison.Ordinal);
    }
}
