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
