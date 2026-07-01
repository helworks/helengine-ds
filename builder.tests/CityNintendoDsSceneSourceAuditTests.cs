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

        Assert.Contains("LayoutComponent nameAnchorComponent = new LayoutComponent {", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("nameAnchorComponent.SetAnchorDistances(left: 8f, bottom: 8f);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("LayoutComponent versionAnchorComponent = new LayoutComponent {", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("versionAnchorComponent.SetAnchorDistances(right: 8f, bottom: 8f);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsTextEntity(entity, \"DemoDiscPlatformInfoNameText\", new float3(8f, 148f, 0f), string.Empty, definition.BodyFontPath, definition.TextColor, new int2(1, 1), 42, nameAnchorComponent, 0.84f, false);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsTextEntity(entity, \"DemoDiscPlatformInfoVersionText\", new float3(176f, 148f, 0f), string.Empty, definition.BodyFontPath, definition.MutedTextColor, new int2(72, 1), 42, versionAnchorComponent, 0.84f, false);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("float3 PlatformNameBaseLocalPosition;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("float3 PlatformVersionBaseLocalPosition;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("int2 PlatformNameBaseSize;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("int2 PlatformVersionBaseSize;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("bool useHorizontalRowLayout = Math.Abs(PlatformNameBaseLocalPosition.Y - PlatformVersionBaseLocalPosition.Y) < 0.01f;", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("ApplyHorizontalText(PlatformNameTextEntity, PlatformNameTextComponent, PlatformNameLayoutComponent, Core.Instance.PlatformInfo.Name, PlatformNameBaseLocalPosition.X, PlatformNameBaseLocalPosition.Y, PlatformNameBaseSize, TextAlignment.Left);", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("ApplyHorizontalText(PlatformVersionTextEntity, PlatformVersionTextComponent, PlatformVersionLayoutComponent, Core.Instance.PlatformInfo.Version, PlatformVersionBaseLocalPosition.X, PlatformVersionBaseLocalPosition.Y, PlatformVersionBaseSize, TextAlignment.Right);", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("textComponent.Size = new int2(", platformInfoComponentSource, StringComparison.Ordinal);
        Assert.Contains("Math.Max(baseSize.X, (int)Math.Ceiling(measuredSize.X * fontScale))", platformInfoComponentSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS main-menu text path uses the generated DS debug font reference instead of the oversized shared body-font file path.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringDsMainMenu_useGeneratedDsDebugFontReference() {
        string mainMenuFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscMainMenuSceneFactory.cs"));

        Assert.Contains("const string NintendoDsDebugFontProviderId = \"editor\";", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("const string NintendoDsDebugFontAssetId = \"ds-debug-font\";", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsTextEntity(", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("BuildNintendoDsDebugFontReference()", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("CreateTextEntity(entity, \"DemoDiscPlatformInfoNameText\", new float3(0f, 0f, 0f), string.Empty, definition.BodyFontPath", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("CreateTextEntity(entity, \"DemoDiscPlatformInfoVersionText\", new float3(240f, 0f, 0f), string.Empty, definition.BodyFontPath", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("definition.BodyFontPath,\r\n                definition.TextColor,\r\n                new int2(NintendoDsScreenWidth - 16, 14)", mainMenuFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the shared Nintendo DS rendering scaffold and bottom instruction overlay persist the generated DS debug-font reference instead of the generic editor default font.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringDsRenderingCompanionScenes_useGeneratedDsDebugFontReferenceForBottomText() {
        string scaffoldFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));
        string instructionOverlayFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "DemoSceneInstructionOverlayFactory.cs"));

        Assert.Contains("const string NintendoDsDebugFontAssetId = \"ds-debug-font\";", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("BuildNintendoDsDebugFontReference()", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("ApplyFontReference(debugRootEntity, debugComponent, BuildNintendoDsDebugFontReference());", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("ApplyFontReference(textEntity, textComponent, BuildNintendoDsDebugFontReference());", scaffoldFactorySource, StringComparison.Ordinal);

        Assert.Contains("const string NintendoDsDebugFontAssetId = \"ds-debug-font\";", instructionOverlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("BuildNintendoDsDebugFontReference()", instructionOverlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("ApplyFontReference(textEntity, textComponent, BuildNintendoDsDebugFontReference());", instructionOverlayFactorySource, StringComparison.Ordinal);
        Assert.Contains("saveComponent.SetAssetReference(component, \"Font\", DemoDiscSceneComponentRecordFactory.CreateEditorFontReference());", instructionOverlayFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS scaffold strips desktop-only instruction and light-toggle UI from the top screen so companion scenes keep 3D content unobstructed.
    /// </summary>
    [Fact]
    public void Sources_whenScaffoldingDsTopScreen_stripDesktopInstructionAndLightToggleUi() {
        string scaffoldFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));

        Assert.Contains("Entity[] filteredTopScreenRoots = FilterTopScreenRoots(topScreenRoots);", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("ConfigureTopScreenRoots(filteredTopScreenRoots);", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("return CombineSceneRoots(filteredTopScreenRoots, bottomScreenCameraEntity);", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("Entity[] FilterTopScreenRoots(Entity[] topScreenRoots)", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("rootEntity is EditorEntity editorRoot", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("string.Equals(editorRoot.Name, \"DemoSceneInstructionViewport\", StringComparison.Ordinal)", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("RemoveLightToggleComponents(entity);", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("RemoveLightIndicatorOverlays(entity);", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("if (entity.Components[componentIndex] is not DemoDiscLightToggleComponent lightToggleComponent)", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("childEntity is EditorEntity editorChild", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("string.Equals(editorChild.Name, DemoDiscLightIndicatorOverlayFactory.IndicatorViewportEntityName, StringComparison.Ordinal)", scaffoldFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS rendering scaffold keeps generated bottom-scene entities on the authored scene-object layer so the scene save pipeline persists them.
    /// </summary>
    [Fact]
    public void Sources_whenRelocatingFpsToDsBottomScreen_assignPersistedSceneLayerMaskToViewportAndFpsHosts() {
        string scaffoldFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));

        Assert.Contains("Entity bottomScreenViewportRoot = Core.Instance.EntityFactory.CreateChild(bottomScreenCameraEntity, \"DemoDiscBottomScreenRoot\");", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("const ushort PersistedSceneLayerMask = EditorLayerMasks.SceneObjects;", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("bottomScreenViewportRoot.LayerMask = PersistedSceneLayerMask;", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("Entity fpsEntity = fpsIndex == 0", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("fpsEntity.LayerMask = PersistedSceneLayerMask;", scaffoldFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the shared DS bottom scaffold now emits the temporary text label plus one visible touchable return button.
    /// </summary>
    [Fact]
    public void Sources_whenAddingDsBottomReturnButton_emitVisibleTouchableBackOverlay() {
        string scaffoldFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));
        string instructionOverlayFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "DemoSceneInstructionOverlayFactory.cs"));

        int createSceneRootsMethodStart = scaffoldFactorySource.IndexOf("public Entity[] CreateSceneRoots(Entity[] topScreenRoots, bool useDefaultBottomOverlay, Entity[] bottomScreenRoots, FontAsset bottomOverlayFont)", StringComparison.Ordinal);
        int createSceneRootsMethodEnd = scaffoldFactorySource.IndexOf("Entity[] FilterTopScreenRoots(Entity[] topScreenRoots)", StringComparison.Ordinal);
        string createSceneRootsMethodBody = scaffoldFactorySource[createSceneRootsMethodStart..createSceneRootsMethodEnd];

        int scaffoldMethodStart = scaffoldFactorySource.IndexOf("void CreateDefaultBottomOverlay(Entity bottomScreenViewportRoot, FontAsset bottomOverlayFont)", StringComparison.Ordinal);
        int scaffoldMethodEnd = scaffoldFactorySource.IndexOf("void CreateBottomScreenBackButton(Entity bottomScreenViewportRoot, FontAsset bottomOverlayFont)", StringComparison.Ordinal);
        string scaffoldMethodBody = scaffoldFactorySource[scaffoldMethodStart..scaffoldMethodEnd];

        int instructionMethodStart = instructionOverlayFactorySource.IndexOf("public Entity[] CreateNintendoDsBottomInstructionRoots(FontAsset font)", StringComparison.Ordinal);
        int instructionMethodEnd = instructionOverlayFactorySource.IndexOf("void CreateDesktopInstructionRow(", StringComparison.Ordinal);
        string instructionMethodBody = instructionOverlayFactorySource[instructionMethodStart..instructionMethodEnd];

        Assert.Contains("CreateBottomScreenBackButton(bottomScreenViewportRoot, bottomOverlayFont);", createSceneRootsMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("if (useDefaultBottomOverlay) {\r\n                CreateBottomScreenBackButton(bottomScreenViewportRoot, bottomOverlayFont);", createSceneRootsMethodBody, StringComparison.Ordinal);
        Assert.Contains("DemoDiscBottomScreenTestText", scaffoldMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("BOTTOM TEXT", scaffoldMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("DemoDiscBottomScreenBackButton", scaffoldMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("DemoDiscBottomScreenDebugRoot", scaffoldMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("new DebugComponent()", scaffoldMethodBody, StringComparison.Ordinal);
        Assert.Contains("return Array.Empty<Entity>();", instructionMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("CreateNintendoDsInstructionRow(", instructionMethodBody, StringComparison.Ordinal);
        Assert.DoesNotContain("new RoundedRectComponent", instructionMethodBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS bottom scaffold uses one shared serialized font reference for its FPS, proof text, and back-button label so the bottom BG0 glyph cache does not thrash between multiple atlases.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringDsBottomOverlay_useSingleEditorUiFontReferenceAcrossBottomText() {
        string scaffoldFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));

        Assert.Contains("ApplyFontReference(fpsEntity, bottomScreenFpsComponent, DemoDiscSceneComponentRecordFactory.CreateEditorUiFontReference());", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("ApplyFontReference(textEntity, textComponent, DemoDiscSceneComponentRecordFactory.CreateEditorUiFontReference());", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("ApplyFontReference(backButtonLabelEntity, labelComponent, DemoDiscSceneComponentRecordFactory.CreateEditorUiFontReference());", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("ApplyFontReference(textEntity, textComponent, DemoDiscSceneComponentRecordFactory.CreateEditorFontReference());", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("ApplyFontReference(backButtonLabelEntity, labelComponent, DemoDiscSceneComponentRecordFactory.CreateEditorFontReference());", scaffoldFactorySource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS main-menu bottom screen restores real item labels through authored text while keeping the row chrome visible behind the BG0 label text.
    /// </summary>
    [Fact]
    public void Sources_whenRestoringDsBottomMenuLabels_keepItemChromeVisibleBehindAuthoredDsTextLabels() {
        string mainMenuFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoDiscMainMenuSceneFactory.cs"));
        int methodStart = mainMenuFactorySource.IndexOf("Entity CreateNintendoDsBottomScreenCameraEntity(string providerTypeName, MenuDefinition definition) {", StringComparison.Ordinal);
        int methodEnd = mainMenuFactorySource.IndexOf("CameraClearSettings BuildNintendoDsCameraClearSettings(byte4 clearColor) {", StringComparison.Ordinal);
        string methodBody = mainMenuFactorySource[methodStart..methodEnd];

        Assert.Contains("CreateNintendoDsPanelEntity(generatedRootEntity, definition, definition.Panels[panelIndex]);", methodBody, StringComparison.Ordinal);
        Assert.Contains("const ushort NintendoDsMenuMetadataLayerMask = EditorLayerMasks.SceneObjects;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("panelEntity.LocalPosition = new float3(0f, 0f, 0f);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("itemsViewportEntity.LocalPosition = new float3(0f, 6f, 0f);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("itemsRootLayoutComponent.SetAnchorDistances(left: 0f, top: 0f, bottom: ItemsViewportTop);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("itemsRootEntity.AddComponent(itemsRootLayoutComponent);", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("itemEntity.LayerMask = NintendoDsMenuMetadataLayerMask;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("itemEntity.LayerMask = RuntimeLayerMask;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("itemsRootEntity.LayerMask = NintendoDsMenuMetadataLayerMask;", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("LayerMask = RuntimeLayerMask", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("LayerMask = (byte)NintendoDsMenuMetadataLayerMask", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("CreateNintendoDsTextEntity(", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("itemDefinition.Label", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.Contains("\"Item-\" + itemDefinition.ItemId + \"-Label\"", mainMenuFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("CreateNintendoDsBottomScreenTextEntity(generatedRootEntity, definition);", methodBody, StringComparison.Ordinal);
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
        string normalizedDsPhysicsSceneGeneratorSource = dsPhysicsSceneGeneratorSource.Replace("\r\n", "\n", StringComparison.Ordinal);
        Assert.Contains("SceneWriteService.WriteNintendoDsCompanionScene(", dsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("topScreenRoots,\n                    true,", normalizedDsPhysicsSceneGeneratorSource, StringComparison.Ordinal);
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
    /// Verifies the demo-disc menu labels the render-only probe as Matrix Probe and preserves the renamed authored scene ids.
    /// </summary>
    [Fact]
    public void Sources_whenMenuExposesRenderProbe_labelItAsRenderMatrixProbe() {
        string demoDiscSceneCatalogSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu", "DemoDiscSceneCatalog.cs"));

        Assert.Contains("new MenuItemDefinition(\"scene-render-matrix-probe\", \"Matrix Probe\", true, new MenuActionDefinition(MenuActionKind.LoadScene, \"test_scene_render_matrix_probe\"))", demoDiscSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("\"test_scene_render_matrix_probe_ds\"", demoDiscSceneCatalogSource, StringComparison.Ordinal);
        Assert.DoesNotContain("new MenuItemDefinition(\"scene-render-matrix-probe\", \"Render Motion Probe\"", demoDiscSceneCatalogSource, StringComparison.Ordinal);
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

        SceneAsset stackedBoxesDsSceneAsset = ReadSceneAsset(Path.Combine("physics", "test_scene_dynamic_stack_boxes_ds.helen"));
        SceneEntityAsset bottomScreenCamera = FindRootEntity(stackedBoxesDsSceneAsset, "DemoDiscBottomScreenCamera");
        SceneEntityAsset bottomScreenRoot = FindChildEntity(bottomScreenCamera, "DemoDiscBottomScreenRoot");
        SceneEntityAsset debugRoot = FindChildEntity(bottomScreenRoot, "DemoDiscBottomScreenDebugRoot");
        SceneComponentAssetRecord debugComponent = Assert.Single(debugRoot.Components, component => string.Equals(component.ComponentTypeId, "helengine.DebugComponent", StringComparison.Ordinal));
        Assert.Equal(1f, ReadDebugComponentFontScale(debugComponent));
    }

    /// <summary>
    /// Verifies the committed DS companion scenes were regenerated for the current text-isolation pass and no longer contain the legacy bottom debug root or back-button overlay.
    /// </summary>
    [Fact]
    public void Assets_whenIsolatingDsBottomText_keepOnlySingleBottomTextEntityInCommittedDsScenes() {
        string[] dsScenePaths = [
            Path.Combine(CityProjectRootPath, "assets", "scenes", "rendering", "ds", "scaled_cube_ds.helen"),
            Path.Combine(CityProjectRootPath, "assets", "scenes", "rendering", "ds", "scene_memory_probe_ds.helen"),
            Path.Combine(CityProjectRootPath, "assets", "scenes", "physics", "test_scene_dynamic_mixed_stack_ds.helen")
        ];

        for (int index = 0; index < dsScenePaths.Length; index++) {
            Assert.True(File.Exists(dsScenePaths[index]));
            string sceneSource = Encoding.ASCII.GetString(File.ReadAllBytes(dsScenePaths[index]));
            Assert.Contains("DemoDiscBottomScreenTestText", sceneSource, StringComparison.Ordinal);
            Assert.DoesNotContain("DemoDiscBottomScreenDebugRoot", sceneSource, StringComparison.Ordinal);
            Assert.DoesNotContain("DemoDiscBottomScreenBackButton", sceneSource, StringComparison.Ordinal);
            Assert.DoesNotContain("NintendoDsReturnOverlayComponent", sceneSource, StringComparison.Ordinal);
        }
    }

    /// <summary>
    /// Verifies the committed DS main-menu scene restores visible item labels while keeping the baked menu metadata required at runtime.
    /// </summary>
    [Fact]
    public void Assets_whenRestoringDsBottomMenuLabels_preservePanelMetadataAndRealMenuText() {
        string dsMenuScenePath = Path.Combine(CityProjectRootPath, "assets", "scenes", "DemoDiscMainMenuDs.helen");

        Assert.True(File.Exists(dsMenuScenePath));
        string dsMenuSceneSource = Encoding.ASCII.GetString(File.ReadAllBytes(dsMenuScenePath));

        Assert.DoesNotContain("DemoDiscBottomScreenTestText", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.DoesNotContain("BOTTOM TEXT", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("Demo Scenes", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("Item-main-scenes-Label", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("Item-main-physics-Label", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("Physics Scenes", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("Panel-main", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("Item-main-scenes", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("city.menu.MenuPanelComponent, gameplay", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("city.menu.MenuItemComponent, gameplay", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("helengine.TextComponent", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("helengine.ScrollComponent", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("helengine.ClipRectComponent", dsMenuSceneSource, StringComparison.Ordinal);
        Assert.Contains("helengine.RoundedRectComponent", dsMenuSceneSource, StringComparison.Ordinal);

        SceneAsset dsMenuSceneAsset = ReadSceneAsset("DemoDiscMainMenuDs.helen");
        SceneEntityAsset panelEntity = FindRequiredEntityRecursive(dsMenuSceneAsset.RootEntities, "Panel-main");
        Assert.Equal(0f, panelEntity.LocalPosition.Y);
        SceneEntityAsset itemsViewportEntity = FindRequiredEntityRecursive(dsMenuSceneAsset.RootEntities, "Panel-main-ItemsViewport");
        Assert.Equal(6f, itemsViewportEntity.LocalPosition.Y);
        SceneEntityAsset itemsRootEntity = FindRequiredEntityRecursive(dsMenuSceneAsset.RootEntities, "Panel-main-ItemsRoot");
        Assert.DoesNotContain(
            itemsRootEntity.Components ?? Array.Empty<SceneComponentAssetRecord>(),
            component => string.Equals(component.ComponentTypeId, "helengine.LayoutComponent", StringComparison.Ordinal));
        Assert.DoesNotContain(
            itemsViewportEntity.Components ?? Array.Empty<SceneComponentAssetRecord>(),
            component => string.Equals(component.ComponentTypeId, "helengine.LayoutComponent", StringComparison.Ordinal));
        SceneEntityAsset firstItemEntity = FindRequiredEntityRecursive(dsMenuSceneAsset.RootEntities, "Item-main-scenes");
        Assert.Equal<ushort>(0b0100000000000000, firstItemEntity.LayerMask);
        SceneComponentAssetRecord roundedRectRecord = Assert.Single(
            firstItemEntity.Components,
            component => string.Equals(component.ComponentTypeId, "helengine.RoundedRectComponent", StringComparison.Ordinal));
        Assert.Equal(1, ReadTaggedByteField(roundedRectRecord, "LayerMask"));
    }

    /// <summary>
    /// Verifies the committed generated DS rendering scenes relocate their authored FPS overlay to the scaffold-owned bottom-screen viewport.
    /// </summary>
    [Fact]
    public void Assets_whenGeneratedDsRenderingScenesMoveFpsToBottomScreen_includeBottomScreenFpsComponent() {
        string[] sceneRelativePaths = [
            Path.Combine("rendering", "ds", "cube_test_ds.helen"),
            Path.Combine("rendering", "ds", "scaled_cube_ds.helen")
        ];

        for (int index = 0; index < sceneRelativePaths.Length; index++) {
            SceneAsset sceneAsset = ReadSceneAsset(sceneRelativePaths[index]);
            SceneEntityAsset bottomScreenCamera = FindRootEntity(sceneAsset, "DemoDiscBottomScreenCamera");
            SceneEntityAsset bottomScreenRoot = FindChildEntity(bottomScreenCamera, "DemoDiscBottomScreenRoot");
            SceneComponentAssetRecord fpsComponent = FindRequiredBottomScreenFpsComponent(bottomScreenRoot);
            Assert.Equal(2f, ReadTaggedFloatField(fpsComponent, "FontScale"));
        }
    }

    /// <summary>
    /// Verifies the committed render-matrix-probe DS companion scene includes one relocated bottom-screen FPS overlay plus the scaffold-owned return button.
    /// </summary>
    [Fact]
    public void Assets_whenRenderMotionProbeDsSceneIsGenerated_includeBottomScreenFpsAndBackButton() {
        SceneAsset sceneAsset = ReadSceneAsset(Path.Combine("physics", "test_scene_render_matrix_probe_ds.helen"));
        SceneEntityAsset bottomScreenCamera = FindRootEntity(sceneAsset, "DemoDiscBottomScreenCamera");
        SceneEntityAsset bottomScreenRoot = FindChildEntity(bottomScreenCamera, "DemoDiscBottomScreenRoot");
        SceneComponentAssetRecord fpsComponent = FindRequiredBottomScreenFpsComponent(bottomScreenRoot);
        SceneEntityAsset backButtonEntity = FindRequiredEntityRecursive(bottomScreenRoot.Children, "DemoDiscBottomScreenBackButton");

        Assert.Equal(2f, ReadTaggedFloatField(fpsComponent, "FontScale"));
        Assert.Contains(
            backButtonEntity.Components ?? Array.Empty<SceneComponentAssetRecord>(),
            component => string.Equals(component.ComponentTypeId, "helengine.InteractableComponent", StringComparison.Ordinal));
        Assert.Contains(
            backButtonEntity.Components ?? Array.Empty<SceneComponentAssetRecord>(),
            component => string.Equals(component.ComponentTypeId, "city.menu.NintendoDsReturnOverlayComponent, gameplay", StringComparison.Ordinal));
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
    /// Reads one generated city scene asset from the committed project scene folder.
    /// </summary>
    /// <param name="sceneRelativePath">Scene path relative to the city project scenes folder.</param>
    /// <returns>Deserialized scene asset.</returns>
    static SceneAsset ReadSceneAsset(string sceneRelativePath) {
        string scenePath = Path.Combine(CityProjectRootPath, "assets", "scenes", sceneRelativePath);
        Assert.True(File.Exists(scenePath));

        byte[] sceneBytes = File.ReadAllBytes(scenePath);
        return Assert.IsType<SceneAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(sceneBytes));
    }

    /// <summary>
    /// Reads the serialized debug-component font scale from one generated scene component payload.
    /// </summary>
    /// <param name="component">Debug component record being inspected.</param>
    /// <returns>Serialized font scale value.</returns>
    static float ReadDebugComponentFontScale(SceneComponentAssetRecord component) {
        if (component == null) {
            throw new ArgumentNullException(nameof(component));
        }

        using MemoryStream stream = new MemoryStream(component.Payload ?? Array.Empty<byte>(), false);
        using EngineBinaryReader reader = EngineBinaryReader.Create(stream, EngineBinaryEndianness.LittleEndian);
        byte version = reader.ReadByte();
        if (version != 1) {
            throw new InvalidOperationException("Unexpected editor tagged component payload version.");
        }

        int fieldCount = reader.ReadInt32();
        for (int index = 0; index < fieldCount; index++) {
            string fieldName = reader.ReadString();
            byte[] fieldPayload = reader.ReadByteArray() ?? Array.Empty<byte>();
            if (!string.Equals(fieldName, "FontScale", StringComparison.Ordinal)) {
                continue;
            }

            using MemoryStream fieldStream = new MemoryStream(fieldPayload, false);
            using EngineBinaryReader fieldReader = EngineBinaryReader.Create(fieldStream, EngineBinaryEndianness.LittleEndian);
            return fieldReader.ReadSingle();
        }

        throw new InvalidOperationException("Generated debug component did not serialize FontScale.");
    }

    /// <summary>
    /// Reads one serialized byte field from a generated tagged component payload.
    /// </summary>
    /// <param name="component">Tagged component record being inspected.</param>
    /// <param name="fieldName">Stable field name to resolve.</param>
    /// <returns>Serialized byte field value.</returns>
    static byte ReadTaggedByteField(SceneComponentAssetRecord component, string fieldName) {
        if (component == null) {
            throw new ArgumentNullException(nameof(component));
        } else if (string.IsNullOrWhiteSpace(fieldName)) {
            throw new ArgumentException("Field name must be provided.", nameof(fieldName));
        }

        using MemoryStream stream = new MemoryStream(component.Payload ?? Array.Empty<byte>(), false);
        using EngineBinaryReader reader = EngineBinaryReader.Create(stream, EngineBinaryEndianness.LittleEndian);
        byte version = reader.ReadByte();
        if (version != 1) {
            throw new InvalidOperationException("Unexpected editor tagged component payload version.");
        }

        int fieldCount = reader.ReadInt32();
        for (int index = 0; index < fieldCount; index++) {
            string serializedFieldName = reader.ReadString();
            byte[] fieldPayload = reader.ReadByteArray() ?? Array.Empty<byte>();
            if (!string.Equals(serializedFieldName, fieldName, StringComparison.Ordinal)) {
                continue;
            }

            using MemoryStream fieldStream = new MemoryStream(fieldPayload, false);
            using EngineBinaryReader fieldReader = EngineBinaryReader.Create(fieldStream, EngineBinaryEndianness.LittleEndian);
            return fieldReader.ReadByte();
        }

        throw new InvalidOperationException($"Generated component did not serialize byte field '{fieldName}'.");
    }

    /// <summary>
    /// Reads one serialized float field from a generated tagged component payload.
    /// </summary>
    /// <param name="component">Tagged component record being inspected.</param>
    /// <param name="fieldName">Stable field name to resolve.</param>
    /// <returns>Serialized float field value.</returns>
    static float ReadTaggedFloatField(SceneComponentAssetRecord component, string fieldName) {
        if (component == null) {
            throw new ArgumentNullException(nameof(component));
        } else if (string.IsNullOrWhiteSpace(fieldName)) {
            throw new ArgumentException("Field name must be provided.", nameof(fieldName));
        }

        using MemoryStream stream = new MemoryStream(component.Payload ?? Array.Empty<byte>(), false);
        using EngineBinaryReader reader = EngineBinaryReader.Create(stream, EngineBinaryEndianness.LittleEndian);
        byte version = reader.ReadByte();
        if (version != 1) {
            throw new InvalidOperationException("Unexpected editor tagged component payload version.");
        }

        int fieldCount = reader.ReadInt32();
        for (int index = 0; index < fieldCount; index++) {
            string serializedFieldName = reader.ReadString();
            byte[] fieldPayload = reader.ReadByteArray() ?? Array.Empty<byte>();
            if (!string.Equals(serializedFieldName, fieldName, StringComparison.Ordinal)) {
                continue;
            }

            using MemoryStream fieldStream = new MemoryStream(fieldPayload, false);
            using EngineBinaryReader fieldReader = EngineBinaryReader.Create(fieldStream, EngineBinaryEndianness.LittleEndian);
            return fieldReader.ReadSingle();
        }

        throw new InvalidOperationException($"Generated component did not serialize float field '{fieldName}'.");
    }

    /// <summary>
    /// Finds one root scene entity with the supplied stable name.
    /// </summary>
    /// <param name="sceneAsset">Scene asset being inspected.</param>
    /// <param name="entityName">Stable entity name to resolve.</param>
    /// <returns>Resolved root scene entity.</returns>
    static SceneEntityAsset FindRootEntity(SceneAsset sceneAsset, string entityName) {
        if (sceneAsset == null) {
            throw new ArgumentNullException(nameof(sceneAsset));
        } else if (string.IsNullOrWhiteSpace(entityName)) {
            throw new ArgumentException("Entity name must be provided.", nameof(entityName));
        }

        SceneEntityAsset rootEntity = Assert.Single(
            sceneAsset.RootEntities,
            entity => string.Equals(entity.Name, entityName, StringComparison.Ordinal));
        return rootEntity;
    }

    /// <summary>
    /// Finds one direct child entity with the supplied stable name.
    /// </summary>
    /// <param name="parentEntity">Parent entity that owns the child.</param>
    /// <param name="entityName">Stable child entity name to resolve.</param>
    /// <returns>Resolved child scene entity.</returns>
    static SceneEntityAsset FindChildEntity(SceneEntityAsset parentEntity, string entityName) {
        if (parentEntity == null) {
            throw new ArgumentNullException(nameof(parentEntity));
        } else if (string.IsNullOrWhiteSpace(entityName)) {
            throw new ArgumentException("Entity name must be provided.", nameof(entityName));
        }

        SceneEntityAsset childEntity = Assert.Single(
            parentEntity.Children,
            entity => string.Equals(entity.Name, entityName, StringComparison.Ordinal));
        return childEntity;
    }

    /// <summary>
    /// Finds one required entity anywhere inside the supplied subtree set.
    /// </summary>
    /// <param name="entities">Subtree roots that should be searched recursively.</param>
    /// <param name="entityName">Stable entity name to resolve.</param>
    /// <returns>Resolved entity.</returns>
    static SceneEntityAsset FindRequiredEntityRecursive(SceneEntityAsset[] entities, string entityName) {
        if (entities == null) {
            throw new ArgumentNullException(nameof(entities));
        } else if (string.IsNullOrWhiteSpace(entityName)) {
            throw new ArgumentException("Entity name must be provided.", nameof(entityName));
        }

        for (int index = 0; index < entities.Length; index++) {
            SceneEntityAsset entity = FindEntityRecursive(entities[index], entityName);
            if (entity != null) {
                return entity;
            }
        }

        throw new InvalidOperationException($"Entity '{entityName}' was not found in the supplied subtree.");
    }

    /// <summary>
    /// Finds the serialized bottom-screen FPS component inside one generated Nintendo DS bottom viewport subtree.
    /// </summary>
    /// <param name="bottomScreenRoot">Bottom-screen viewport root that should own the relocated FPS overlay.</param>
    /// <returns>Serialized FPS component record.</returns>
    static SceneComponentAssetRecord FindRequiredBottomScreenFpsComponent(SceneEntityAsset bottomScreenRoot) {
        if (bottomScreenRoot == null) {
            throw new ArgumentNullException(nameof(bottomScreenRoot));
        }

        SceneComponentAssetRecord rootFpsComponent = bottomScreenRoot.Components.SingleOrDefault(
            component => string.Equals(component.ComponentTypeId, "helengine.FPSComponent", StringComparison.Ordinal));
        if (rootFpsComponent != null) {
            return rootFpsComponent;
        }

        SceneEntityAsset bottomScreenFps = FindRequiredEntityRecursive(bottomScreenRoot.Children, "DemoDiscBottomScreenFps");
        return Assert.Single(
            bottomScreenFps.Components,
            component => string.Equals(component.ComponentTypeId, "helengine.FPSComponent", StringComparison.Ordinal));
    }

    /// <summary>
    /// Finds one entity with the supplied stable name inside the provided subtree.
    /// </summary>
    /// <param name="entity">Current subtree entity being inspected.</param>
    /// <param name="entityName">Stable entity name to resolve.</param>
    /// <returns>Resolved entity, or null when no match exists in the subtree.</returns>
    static SceneEntityAsset FindEntityRecursive(SceneEntityAsset entity, string entityName) {
        if (entity == null) {
            return null;
        } else if (string.Equals(entity.Name, entityName, StringComparison.Ordinal)) {
            return entity;
        } else if (entity.Children == null) {
            return null;
        }

        for (int index = 0; index < entity.Children.Length; index++) {
            SceneEntityAsset resolvedEntity = FindEntityRecursive(entity.Children[index], entityName);
            if (resolvedEntity != null) {
                return resolvedEntity;
            }
        }

        return null;
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
