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
        string generatedAuthoringWriterSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "GeneratedAuthoringSceneWriteService.cs"));

        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscMainMenuSceneFactory.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscSceneGenerator.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "RegenerateDemoDiscMainMenuCommand.cs")));
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
        Assert.Contains("sceneDefinition.NintendoDsScene != null", generatedAuthoringWriterSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderingSceneScaffoldFactoryValue.CreateSceneRoots", generatedAuthoringWriterSource, StringComparison.Ordinal);
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
    }
}
