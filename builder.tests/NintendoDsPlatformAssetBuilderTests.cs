using helengine;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Results;
using helengine.baseplatform.Targets;
using helengine.ds.builder.tests.Builders;
using helengine.files;
using System.Runtime.Versioning;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS builder metadata exposed to the editor.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
[SupportedOSPlatform("windows")]
public class NintendoDsPlatformAssetBuilderTests {
    /// <summary>
    /// Verifies the builder descriptor and platform definition expose the expected Nintendo DS metadata.
    /// </summary>
    [Fact]
    public void Descriptor_and_definition_expose_ds_metadata() {
        NintendoDsPlatformAssetBuilder builder = new();

        Assert.Equal("helengine.ds.builder", builder.Descriptor.BuilderId);
        Assert.Equal("1.0.1", builder.Descriptor.BuilderVersion);
        Assert.Equal("ds", builder.Descriptor.TargetPlatformId);
        Assert.Equal("ds", builder.Definition.PlatformId);
        Assert.Contains(builder.Definition.BuildProfiles, profile => profile.ProfileId == "ds-default");
        Assert.Contains(builder.Definition.GraphicsProfiles, profile => profile.ProfileId == "ds-main-2d");
        Assert.Contains(builder.Definition.StorageProfiles, profile =>
            profile.ProfileId == "nitrofs-package" &&
            profile.RuntimeSpecializationId == "ds-nitrofs-package");

        PlatformBuildProfileDefinition buildProfile = Assert.Single(
            builder.Definition.BuildProfiles,
            profile => profile.ProfileId == "ds-default");
        Assert.Contains(buildProfile.Settings, setting => setting.SettingId == "startup-top-screen-color");
        Assert.Contains(buildProfile.Settings, setting => setting.SettingId == "startup-bottom-screen-color");

        Assert.Contains(builder.Definition.ComponentSupportRules, supportRule =>
            supportRule.ComponentTypeId == "helengine.meshcomponent" &&
            supportRule.SupportKind == PlatformComponentSupportKind.Transform);
        Assert.Contains(builder.Definition.ComponentSupportRules, supportRule =>
            supportRule.ComponentTypeId == "helengine.fpscomponent" &&
            supportRule.SupportKind == PlatformComponentSupportKind.Transform);
        Assert.Contains(builder.Definition.ComponentSupportRules, supportRule =>
            supportRule.ComponentTypeId == "city.menu.PlatformInfoTextComponent, gameplay" &&
            supportRule.SupportKind == PlatformComponentSupportKind.PassThrough);
        Assert.Contains(builder.Definition.ComponentSupportRules, supportRule =>
            supportRule.ComponentTypeId == "helengine.SceneMapComponent" &&
            supportRule.SupportKind == PlatformComponentSupportKind.Transform);
        Assert.Contains(builder.Definition.AssetCookCapabilities, capability =>
            capability.SourceAssetKind == "texture"
            && capability.TargetArtifactKind == "runtime-texture"
            && capability.OwnershipKind == PlatformAssetCookOwnershipKind.BuilderOwned);
        Assert.Contains(builder.Definition.AssetCookCapabilities, capability =>
            capability.SourceAssetKind == "font-atlas-texture"
            && capability.TargetArtifactKind == "runtime-texture"
            && capability.OwnershipKind == PlatformAssetCookOwnershipKind.BuilderOwned);
    }

    /// <summary>
    /// Verifies the Nintendo DS builder publishes generic texture-format capability metadata for both image textures and font atlas textures.
    /// </summary>
    [Fact]
    public void Descriptor_and_definition_expose_ds_texture_format_capabilities() {
        NintendoDsPlatformAssetBuilder builder = new();

        Assert.Collection(
            builder.Definition.AssetCookCapabilities,
            capability => {
                Assert.Equal("texture", capability.SourceAssetKind);
                Assert.Equal("runtime-texture", capability.TargetArtifactKind);
                Assert.Equal(PlatformAssetCookOwnershipKind.BuilderOwned, capability.OwnershipKind);
                Assert.Equal("ds-texture", capability.SettingsContractId);
                Assert.Equal("{\"maxResolution\":256,\"colorFormat\":\"Rgba4444\",\"alphaPrecision\":\"A4\"}", capability.DefaultSerializedPlatformSettings);
                AssertTextureFormatCapabilities(capability.TextureFormatCapabilities);
            },
            capability => {
                Assert.Equal("font-atlas-texture", capability.SourceAssetKind);
                Assert.Equal("runtime-texture", capability.TargetArtifactKind);
                Assert.Equal(PlatformAssetCookOwnershipKind.BuilderOwned, capability.OwnershipKind);
                Assert.Equal("ds-font-atlas-texture", capability.SettingsContractId);
                Assert.Equal("{\"maxResolution\":256,\"colorFormat\":\"Indexed4\",\"alphaPrecision\":\"Binary\"}", capability.DefaultSerializedPlatformSettings);
                Assert.Equal(".hetex", capability.OutputFileExtension);
                AssertTextureFormatCapabilities(capability.TextureFormatCapabilities);
            });
    }

    /// <summary>
    /// Verifies the Nintendo DS repository-root resolver falls back to the builder assembly location when the current working directory is outside the repository.
    /// </summary>
    [Fact]
    public void ResolveRepositoryRootPath_whenCurrentDirectoryIsOutsideRepository_usesBuilderAssemblyLocation() {
        string originalCurrentDirectory = Directory.GetCurrentDirectory();
        string originalRepositoryRootEnvironmentVariableValue = Environment.GetEnvironmentVariable("HELENGINE_DS_REPOSITORY_ROOT");
        string temporaryDirectoryPath = Path.Combine(Path.GetTempPath(), "helengine-ds-root-resolution-" + Guid.NewGuid().ToString("N"));

        try {
            Environment.SetEnvironmentVariable("HELENGINE_DS_REPOSITORY_ROOT", null);
            Directory.CreateDirectory(temporaryDirectoryPath);
            Directory.SetCurrentDirectory(temporaryDirectoryPath);

            System.Reflection.MethodInfo resolveRepositoryRootPathMethod = typeof(NintendoDsPlatformAssetBuilder).GetMethod(
                "ResolveRepositoryRootPath",
                System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic)
                ?? throw new InvalidOperationException("Unable to resolve the private Nintendo DS repository root helper.");
            string repositoryRootPath = (string)(resolveRepositoryRootPathMethod.Invoke(null, null)
                ?? throw new InvalidOperationException("Nintendo DS repository root resolution returned null."));

            Assert.Equal(Path.GetFullPath("C:\\dev\\helworks\\helengine-ds"), repositoryRootPath);
        } finally {
            Directory.SetCurrentDirectory(originalCurrentDirectory);
            Environment.SetEnvironmentVariable("HELENGINE_DS_REPOSITORY_ROOT", originalRepositoryRootEnvironmentVariableValue);
            if (Directory.Exists(temporaryDirectoryPath)) {
                Directory.Delete(temporaryDirectoryPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the Nintendo DS startup-scene resolver uses the manifest-declared startup scene id instead of hardcoding the generated boot scene id.
    /// </summary>
    [Fact]
    public void FindNintendoDsStartupScene_whenManifestDeclaresCustomStartupScene_returnsDeclaredStartupScene() {
        PlatformBuildManifest manifest = new(
            3,
            "project",
            "1.0.0",
            "1.0.0",
            "ds",
            "1",
            "DemoDiscMainMenu",
            [
                new PlatformBuildScene(
                    "DemoDiscMainMenu",
                    "Demo Disc Main Menu",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenu.hasset", "cooked/scenes/DemoDiscMainMenu.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenu.hasset")])
            ],
            Array.Empty<PlatformBuildAsset>(),
            Array.Empty<PlatformBuildArtifact>(),
            Array.Empty<PlatformBuildCodeModule>(),
            Array.Empty<PlatformArtifactPlacement>(),
            new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()),
            Array.Empty<PlatformCookWorkItem>());

        System.Reflection.MethodInfo findNintendoDsStartupSceneMethod = typeof(NintendoDsPlatformAssetBuilder).GetMethod(
            "FindNintendoDsStartupScene",
            System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic)
            ?? throw new InvalidOperationException("Unable to resolve the private Nintendo DS startup-scene helper.");
        PlatformBuildScene startupScene = (PlatformBuildScene)(findNintendoDsStartupSceneMethod.Invoke(null, [manifest])
            ?? throw new InvalidOperationException("Nintendo DS startup-scene resolution returned null."));

        Assert.Equal("DemoDiscMainMenu", startupScene.SceneId);
    }

    /// <summary>
    /// Verifies the Nintendo DS native build script links the generated runtime scene-catalog manifest when the editor emits it.
    /// </summary>
    [Fact]
    public void Makefile_whenGeneratedRuntimeSceneCatalogManifestExists_linksSceneCatalogSource() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefilePath = Path.Combine(repositoryRootPath, "Makefile");
        string makefileSource = File.ReadAllText(makefilePath);

        Assert.Contains("runtime_scene_catalog_manifest.cpp", makefileSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS native build script enables C++20 so generated-core runtime headers can use concepts-based helpers.
    /// </summary>
    [Fact]
    public void Makefile_whenBuildingGeneratedCore_enablesCpp20ForNativeRuntimeHeaders() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefilePath = Path.Combine(repositoryRootPath, "Makefile");
        string makefileSource = File.ReadAllText(makefilePath);

        Assert.Contains("CXXFLAGS := $(CFLAGS) -std=gnu++20", makefileSource, StringComparison.Ordinal);
        Assert.DoesNotContain("CXXFLAGS := $(CFLAGS) -std=gnu++17", makefileSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS builder exposes the cooked-platform-owned material contract and at least one material schema.
    /// </summary>
    [Fact]
    public void Definition_exposes_cooked_platform_owned_material_contract_and_material_schemas() {
        NintendoDsPlatformAssetBuilder builder = new();

        Assert.Equal(RuntimeMaterialResolutionMode.CookedPlatformOwned, builder.Definition.RuntimeGenerationContract.MaterialResolutionMode);
        Assert.NotEmpty(builder.Definition.MaterialSchemas);
        Assert.Contains(builder.Definition.MaterialSchemas, schema => schema.SchemaId == "ds-standard-textured");
    }

    /// <summary>
    /// Verifies one Nintendo DS texture cook capability advertises the expected supported formats and valid combinations.
    /// </summary>
    /// <param name="textureFormatCapabilities">Texture capability metadata to validate.</param>
    static void AssertTextureFormatCapabilities(PlatformTextureFormatCapabilityDefinition textureFormatCapabilities) {
        Assert.NotNull(textureFormatCapabilities);
        string[] expectedColorFormatIds = [
            TextureAssetColorFormat.Rgba4444.ToString(),
            TextureAssetColorFormat.Indexed4.ToString(),
            TextureAssetColorFormat.Indexed8.ToString()
        ];
        Assert.Equal(
            expectedColorFormatIds,
            textureFormatCapabilities.SupportedColorFormatIds);
        Assert.Equal(
            [TextureAssetAlphaPrecision.Binary, TextureAssetAlphaPrecision.A4, TextureAssetAlphaPrecision.A8],
            textureFormatCapabilities.SupportedAlphaPrecisions);
        Assert.Collection(
            textureFormatCapabilities.SupportedCombinations,
            combination => {
                Assert.Equal(TextureAssetColorFormat.Rgba4444.ToString(), combination.ColorFormatId);
                Assert.Equal(TextureAssetAlphaPrecision.A4, combination.AlphaPrecision);
            },
            combination => {
                Assert.Equal(TextureAssetColorFormat.Indexed4.ToString(), combination.ColorFormatId);
                Assert.Equal(TextureAssetAlphaPrecision.Binary, combination.AlphaPrecision);
            },
            combination => {
                Assert.Equal(TextureAssetColorFormat.Indexed8.ToString(), combination.ColorFormatId);
                Assert.Equal(TextureAssetAlphaPrecision.A8, combination.AlphaPrecision);
            });
    }

    /// <summary>
    /// Verifies the Nintendo DS builder cooks platform-owned material payloads without shader references.
    /// </summary>
    [Fact]
    public void CookMaterial_whenUsingDsStandardSchema_returnsPlatformMaterialAsset_withoutShaderReferences() {
        NintendoDsPlatformAssetBuilder builder = new();

        PlatformMaterialCookResult result = builder.CookMaterial(new PlatformMaterialCookRequest(
            "Materials/rendering/test/Cube00",
            "Materials/rendering/test/Cube00.helmat",
            "ds",
            "ds-default",
            "ds-main-2d",
            "ds-standard-textured",
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
                ["texture-relative-path"] = "cooked/imported/test-texture",
                ["double-sided"] = "false",
                ["vertex-color-mode"] = "multiply",
                ["base-color"] = "#FFFFFFFF",
                ["lighting-mode"] = "lit"
            }));

        Assert.Empty(result.ReferencedShaderAssetIds);

        PlatformMaterialAsset cookedAsset = Assert.IsType<PlatformMaterialAsset>(AssetSerializer.DeserializeFromBytes(result.CookedMaterialBytes));
        Assert.Equal("ds-main-2d", cookedAsset.RendererFamilyId);
    }

    /// <summary>
    /// Verifies the build flow stages the startup manifest and delegates native packaging through the executor seam.
    /// </summary>
    [Fact]
    public async Task BuildAsync_writes_startup_manifest_and_invokes_native_executor() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")]),
                new PlatformBuildScene(
                    "DemoDiscMainMenuDs",
                    "Demo Disc Main Menu DS",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenuDs.hasset", "cooked/scenes/DemoDiscMainMenuDs.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenuDs.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            Assert.True(report.Succeeded);
            Assert.NotNull(nativeBuildExecutor.Workspace);
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "runtime", "ds_startup_manifest.bin")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "GeneratedBootScene.hasset")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "DemoDiscMainMenuDs.hasset")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.StagedGeneratedCoreRootPath, "helengine_core_amalgamated.cpp")));
            Assert.True(File.Exists(nativeBuildExecutor.Workspace.ExportPackagePath));
            Assert.Empty(diagnosticReporter.Diagnostics);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the builder sanitizes staged Nintendo DS scene assets before native packaging begins.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenStagedSceneContainsReturnToMenuComponent_preservesSceneAsset() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes", "rendering"));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "rendering", "cube_test.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: true));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "cube_test",
                    "Cube Test",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/rendering/cube_test.hasset", "cooked/scenes/rendering/cube_test.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/rendering/cube_test.hasset")]),
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")]),
                new PlatformBuildScene(
                    "DemoDiscMainMenuDs",
                    "Demo Disc Main Menu DS",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenuDs.hasset", "cooked/scenes/DemoDiscMainMenuDs.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenuDs.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "cube_test",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            string stagedScenePath = Path.Combine(
                nativeBuildExecutor.Workspace!.NitroFsRootPath,
                "cooked",
                "scenes",
                "rendering",
                "cube_test.hasset");
            SceneAsset sanitizedSceneAsset = Assert.IsType<SceneAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(stagedScenePath)));
            SceneEntityAsset rootEntity = Assert.Single(sanitizedSceneAsset.RootEntities);

            Assert.Contains(
                rootEntity.Components,
                component => string.Equals(component.ComponentTypeId, "city.menu.DemoDiscReturnToMenuComponent, gameplay", StringComparison.Ordinal));
            Assert.Contains(
                rootEntity.Components,
                component => string.Equals(component.ComponentTypeId, "helengine.MeshComponent", StringComparison.Ordinal));
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the builder fails fast when the selected startup scene does not stage its cooked payload into NitroFS.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenStartupScenePayloadIsNotStaged_throws_clear_error() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(packageRoot);
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    Array.Empty<PlatformBuildPayloadReference>(),
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            InvalidOperationException exception = await Assert.ThrowsAsync<InvalidOperationException>(() =>
                builder.BuildAsync(
                    request,
                    progressReporter,
                    diagnosticReporter,
                    CancellationToken.None));

            Assert.Equal(
                "Nintendo DS startup scene 'GeneratedBootScene' did not stage cooked payload 'cooked/scenes/GeneratedBootScene.hasset' into NitroFS.",
                exception.Message);
            Assert.Null(nativeBuildExecutor.Workspace);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the builder rejects generated-core roots that do not contain the required editor-produced runtime contract files.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenGeneratedCoreContractIsIncomplete_throws_clear_error_before_native_build() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")]),
                new PlatformBuildScene(
                    "DemoDiscMainMenuDs",
                    "Demo Disc Main Menu DS",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenuDs.hasset", "cooked/scenes/DemoDiscMainMenuDs.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenuDs.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            InvalidOperationException exception = await Assert.ThrowsAsync<InvalidOperationException>(() =>
                builder.BuildAsync(
                    request,
                    progressReporter,
                    diagnosticReporter,
                    CancellationToken.None));

            Assert.Equal(
                "Nintendo DS builds require the editor-finalized generated core output. Generated runtime component registration files were not found.",
                exception.Message);
            Assert.Null(nativeBuildExecutor.Workspace);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the builder clears previously staged NitroFS payloads when the same working root is reused for a later build.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenWorkingRootIsReused_clears_stale_nitrofs_files_before_restaging() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")]),
                new PlatformBuildScene(
                    "DemoDiscMainMenuDs",
                    "Demo Disc Main Menu DS",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenuDs.hasset", "cooked/scenes/DemoDiscMainMenuDs.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenuDs.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            string staleFilePath = Path.Combine(workingRoot, "tmp", "ds", "nitrofs", "stale.bin");
            Directory.CreateDirectory(Path.GetDirectoryName(staleFilePath)
                ?? throw new InvalidOperationException("Unable to resolve the stale NitroFS file directory."));
            File.WriteAllText(staleFilePath, "stale");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            Directory.SetCurrentDirectory(outputRoot);

            await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            Assert.False(File.Exists(staleFilePath));
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the builder stages payloads from the explicit working-root package source instead of the process current directory.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenCurrentDirectoryDiffersFromPackageSource_still_stages_payloads() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string unrelatedCurrentDirectory = Path.Combine(workingRoot, "unrelated-cwd");
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            Directory.CreateDirectory(unrelatedCurrentDirectory);
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.SetCurrentDirectory(unrelatedCurrentDirectory);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")]),
                new PlatformBuildScene(
                    "DemoDiscMainMenuDs",
                    "Demo Disc Main Menu DS",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenuDs.hasset", "cooked/scenes/DemoDiscMainMenuDs.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenuDs.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            Assert.True(report.Succeeded);
            Assert.NotNull(nativeBuildExecutor.Workspace);
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "GeneratedBootScene.hasset")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "DemoDiscMainMenuDs.hasset")));
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies Nintendo DS builds stage the generated boot scene as the effective startup scene even when the authored startup scene differs.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenManifestStartupSceneDiffersFromBootSceneContract_usesGeneratedBootSceneAsEffectiveStartupScene() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes", "rendering"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "rendering", "colored_cube_grid.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenuDs.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "colored_cube_grid",
                    "Colored Cube Grid",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/rendering/colored_cube_grid.hasset", "cooked/scenes/rendering/colored_cube_grid.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/rendering/colored_cube_grid.hasset")]),
                new PlatformBuildScene(
                    "GeneratedBootScene",
                    "Generated Boot Scene",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")]),
                new PlatformBuildScene(
                    "DemoDiscMainMenuDs",
                    "Demo Disc Main Menu DS",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenuDs.hasset", "cooked/scenes/DemoDiscMainMenuDs.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenuDs.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "colored_cube_grid",
                scenes,
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            string stagedStartupScenePath = Path.Combine(
                nativeBuildExecutor.Workspace!.NitroFsRootPath,
                "cooked",
                "scenes",
                "GeneratedBootScene.hasset");
            string stagedRuntimeManifestPath = Path.Combine(
                nativeBuildExecutor.Workspace.StagedGeneratedCoreRootPath,
                "runtime",
                "runtime_startup_manifest.cpp");
            Assert.True(File.Exists(stagedStartupScenePath));
            Assert.Contains(
                "cooked/scenes/GeneratedBootScene.hasset",
                File.ReadAllText(stagedRuntimeManifestPath),
                StringComparison.Ordinal);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the Nintendo DS builder executes one builder-owned texture cook work item and stages the cooked runtime texture into NitroFS.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenGivenTexturePlatformCookWorkItem_stagesCookedRuntimeTextureIntoNitroFs() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string sourceTexturePath = Path.Combine(workingRoot, "assets", "Images", "Menu", "helengine-logo.png");
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            Directory.CreateDirectory(Path.GetDirectoryName(sourceTexturePath)
                ?? throw new InvalidOperationException("Unable to resolve the test texture source directory."));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllBytes(sourceTexturePath, [1, 2, 3, 4]);

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            FakeNintendoDsPlatformCookSourceProcessor platformCookSourceProcessor = new(
                new TextureAsset {
                    Width = 2,
                    Height = 1,
                    ColorFormat = TextureAssetColorFormat.Indexed8,
                    AlphaPrecision = TextureAssetAlphaPrecision.A8,
                    Colors = [3, 7],
                    PaletteColors = [10, 20, 30, 255, 40, 50, 60, 255]
                },
                new TextureAsset {
                    Width = 1,
                    Height = 1,
                    ColorFormat = TextureAssetColorFormat.Indexed4,
                    AlphaPrecision = TextureAssetAlphaPrecision.Binary,
                    Colors = [0x10],
                    PaletteColors = [255, 255, 255, 255]
                });
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot, platformCookSourceProcessor);

            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                [
                    new PlatformBuildScene(
                        "GeneratedBootScene",
                        "Generated Boot Scene",
                        "scene",
                        [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                        [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()),
                [
                    new PlatformCookWorkItem(
                        "ds:texture:cooked/imported/b0d3f804",
                        sourceTexturePath,
                        "texture",
                        "ds",
                        "runtime-texture",
                        "cooked/imported/b0d3f804",
                        "runtime-texture:cooked/imported/b0d3f804",
                        "sha256:source",
                        "sha256:settings",
                        NintendoDsTextureCookSettingsSerializer.Serialize(new TextureAssetProcessorSettings {
                            MaxResolution = 0,
                            ColorFormat = TextureAssetColorFormat.Indexed8,
                            AlphaPrecision = TextureAssetAlphaPrecision.A8
                        }),
                        [new PlatformCookWorkItemMetadata("source-asset-id", "b0d3f804")])
                ]);

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            Assert.True(report.Succeeded);
            string stagedTexturePath = Path.Combine(nativeBuildExecutor.Workspace!.NitroFsRootPath, "cooked", "imported", "b0d3f804");
            Assert.True(File.Exists(stagedTexturePath));
            TextureAsset stagedTextureAsset = Assert.IsType<TextureAsset>(AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(stagedTexturePath)));
            Assert.Equal("b0d3f804", stagedTextureAsset.Id);
            Assert.Equal(RuntimeAssetIdGenerator.Generate("b0d3f804"), stagedTextureAsset.RuntimeAssetId);
            Assert.Equal(TextureAssetColorFormat.Indexed8, stagedTextureAsset.ColorFormat);
            Assert.Equal(TextureAssetAlphaPrecision.A8, stagedTextureAsset.AlphaPrecision);
            Assert.Equal([3, 7], stagedTextureAsset.Colors);
            Assert.Equal([10, 20, 30, 255, 40, 50, 60, 255], stagedTextureAsset.PaletteColors);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the Nintendo DS builder executes one builder-owned font-atlas cook work item and stages the cooked runtime atlas texture into NitroFS.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenGivenFontAtlasPlatformCookWorkItem_stagesCookedRuntimeAtlasTextureIntoNitroFs() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string sourceFontPath = Path.Combine(workingRoot, "assets", "Fonts", "DemoDiscTitle.hefont");
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            Directory.CreateDirectory(Path.GetDirectoryName(sourceFontPath)
                ?? throw new InvalidOperationException("Unable to resolve the test font source directory."));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            WriteTestFontAsset(sourceFontPath);

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            FakeNintendoDsPlatformCookSourceProcessor platformCookSourceProcessor = new(
                new TextureAsset {
                    Width = 1,
                    Height = 1,
                    ColorFormat = TextureAssetColorFormat.Rgba4444,
                    AlphaPrecision = TextureAssetAlphaPrecision.A4,
                    Colors = [0xAA, 0x55]
                },
                new TextureAsset {
                    Width = 4,
                    Height = 4,
                    ColorFormat = TextureAssetColorFormat.Indexed4,
                    AlphaPrecision = TextureAssetAlphaPrecision.Binary,
                    Colors = [0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE],
                    PaletteColors = [255, 255, 255, 255, 0, 0, 0, 255]
                });
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot, platformCookSourceProcessor);

            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                [
                    new PlatformBuildScene(
                        "GeneratedBootScene",
                        "Generated Boot Scene",
                        "scene",
                        [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                        [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()),
                [
                    new PlatformCookWorkItem(
                        "ds:font-atlas-texture:cooked/fonts/DemoDiscTitle.hetex",
                        sourceFontPath,
                        "font-atlas-texture",
                        "ds",
                        "runtime-texture",
                        "cooked/fonts/DemoDiscTitle.hetex",
                        "runtime-texture:cooked/fonts/DemoDiscTitle.hetex",
                        "sha256:source",
                        "sha256:settings",
                        NintendoDsTextureCookSettingsSerializer.Serialize(new TextureAssetProcessorSettings {
                            MaxResolution = 0,
                            ColorFormat = TextureAssetColorFormat.Indexed4,
                            AlphaPrecision = TextureAssetAlphaPrecision.Binary
                        }),
                        [new PlatformCookWorkItemMetadata("source-asset-id", "ui-font")])
                ]);

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                progressReporter,
                diagnosticReporter,
                CancellationToken.None);

            Assert.True(report.Succeeded);
            string stagedAtlasTexturePath = Path.Combine(nativeBuildExecutor.Workspace!.NitroFsRootPath, "cooked", "fonts", "DemoDiscTitle.hetex");
            Assert.True(File.Exists(stagedAtlasTexturePath));
            TextureAsset stagedAtlasTextureAsset = Assert.IsType<TextureAsset>(AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(stagedAtlasTexturePath)));
            Assert.Equal("ui-font#atlas", stagedAtlasTextureAsset.Id);
            Assert.Equal(RuntimeAssetIdGenerator.Generate("ui-font#atlas"), stagedAtlasTextureAsset.RuntimeAssetId);
            Assert.Equal(TextureAssetColorFormat.Indexed4, stagedAtlasTextureAsset.ColorFormat);
            Assert.Equal(TextureAssetAlphaPrecision.Binary, stagedAtlasTextureAsset.AlphaPrecision);
            Assert.Equal([0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE], stagedAtlasTextureAsset.Colors);
            Assert.Equal([255, 255, 255, 255, 0, 0, 0, 255], stagedAtlasTextureAsset.PaletteColors);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the Nintendo DS builder prefers the staged packaged font asset over the raw source font when cooking one externalized atlas texture.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenGivenRawFontAtlasWorkItemAndStagedPackagedFont_usesStagedHefontAsCookSource() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string sourceFontPath = Path.Combine(workingRoot, "assets", "Fonts", "DemoDiscTitle.ttf");
        string stagedPackagedFontPath = Path.Combine(packageRoot, "cooked", "fonts", "DemoDiscTitle.hefont");
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            Directory.CreateDirectory(Path.GetDirectoryName(sourceFontPath)
                ?? throw new InvalidOperationException("Unable to resolve the test font source directory."));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllText(sourceFontPath, "raw-font-source");
            WriteTestFontAsset(stagedPackagedFontPath);

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            FakeNintendoDsPlatformCookSourceProcessor platformCookSourceProcessor = new(
                new TextureAsset {
                    Width = 1,
                    Height = 1,
                    ColorFormat = TextureAssetColorFormat.Rgba4444,
                    AlphaPrecision = TextureAssetAlphaPrecision.A4,
                    Colors = [0xAA, 0x55]
                },
                new TextureAsset {
                    Width = 4,
                    Height = 4,
                    ColorFormat = TextureAssetColorFormat.Indexed4,
                    AlphaPrecision = TextureAssetAlphaPrecision.Binary,
                    Colors = [0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE],
                    PaletteColors = [255, 255, 255, 255, 0, 0, 0, 255]
                });
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot, platformCookSourceProcessor);

            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                [
                    new PlatformBuildScene(
                        "GeneratedBootScene",
                        "Generated Boot Scene",
                        "scene",
                        [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                        [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()),
                [
                    new PlatformCookWorkItem(
                        "ds:font-atlas-texture:cooked/fonts/DemoDiscTitle.hetex",
                        sourceFontPath,
                        "font-atlas-texture",
                        "ds",
                        "runtime-texture",
                        "cooked/fonts/DemoDiscTitle.hetex",
                        "runtime-texture:cooked/fonts/DemoDiscTitle.hetex",
                        "sha256:source",
                        "sha256:settings",
                        NintendoDsTextureCookSettingsSerializer.Serialize(new TextureAssetProcessorSettings {
                            MaxResolution = 0,
                            ColorFormat = TextureAssetColorFormat.Indexed4,
                            AlphaPrecision = TextureAssetAlphaPrecision.Binary
                        }),
                        [new PlatformCookWorkItemMetadata("source-asset-id", "ui-font")])
                ]);

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                new RecordingProgressReporter(),
                new RecordingDiagnosticReporter(),
                CancellationToken.None);

            Assert.True(report.Succeeded);
            Assert.Equal(stagedPackagedFontPath, platformCookSourceProcessor.LastFontAtlasSourceAssetPath);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the Nintendo DS builder falls back to the raw source font when the staged packaged font externalizes its atlas texture payload.
    /// </summary>
    [Fact]
    public async Task BuildAsync_whenGivenRawFontAtlasWorkItemAndExternalizedStagedPackagedFont_usesRawSourceFontAsCookSource() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
        string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);
        string sourceFontPath = Path.Combine(workingRoot, "assets", "Fonts", "DemoDiscBody.ttf");
        string stagedPackagedFontPath = Path.Combine(packageRoot, "cooked", "fonts", "DemoDiscBody.hefont");
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            Directory.CreateDirectory(Path.GetDirectoryName(sourceFontPath)
                ?? throw new InvalidOperationException("Unable to resolve the test font source directory."));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
                "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/GeneratedBootScene.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "GeneratedBootScene.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
            File.WriteAllText(sourceFontPath, "raw-font-source");
            WriteExternalizedTestFontAsset(stagedPackagedFontPath, "cooked/fonts/DemoDiscBody.hetex");

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            FakeNintendoDsPlatformCookSourceProcessor platformCookSourceProcessor = new(
                new TextureAsset {
                    Width = 1,
                    Height = 1,
                    ColorFormat = TextureAssetColorFormat.Rgba4444,
                    AlphaPrecision = TextureAssetAlphaPrecision.A4,
                    Colors = [0xAA, 0x55]
                },
                new TextureAsset {
                    Width = 4,
                    Height = 4,
                    ColorFormat = TextureAssetColorFormat.Indexed4,
                    AlphaPrecision = TextureAssetAlphaPrecision.Binary,
                    Colors = [0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE],
                    PaletteColors = [255, 255, 255, 255, 0, 0, 0, 255]
                });
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot, platformCookSourceProcessor);

            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1",
                "GeneratedBootScene",
                [
                    new PlatformBuildScene(
                        "GeneratedBootScene",
                        "Generated Boot Scene",
                        "scene",
                        [new PlatformBuildPayloadReference("cooked/scenes/GeneratedBootScene.hasset", "cooked/scenes/GeneratedBootScene.hasset")],
                        [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/GeneratedBootScene.hasset")])
                ],
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()),
                [
                    new PlatformCookWorkItem(
                        "ds:font-atlas-texture:cooked/fonts/DemoDiscBody.hetex",
                        sourceFontPath,
                        "font-atlas-texture",
                        "ds",
                        "runtime-texture",
                        "cooked/fonts/DemoDiscBody.hetex",
                        "runtime-texture:cooked/fonts/DemoDiscBody.hetex",
                        "sha256:source",
                        "sha256:settings",
                        NintendoDsTextureCookSettingsSerializer.Serialize(new TextureAssetProcessorSettings {
                            MaxResolution = 0,
                            ColorFormat = TextureAssetColorFormat.Indexed4,
                            AlphaPrecision = TextureAssetAlphaPrecision.Binary
                        }),
                        [new PlatformCookWorkItemMetadata("source-asset-id", "ui-font")])
                ]);

            PlatformBuildRequest request = new(
                manifest,
                [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
                [new PlatformCookProfile(
                    "ds-default",
                    "DS Default",
                    new PlatformCookProfileCapabilities(
                        "ds",
                        "raw",
                        "raw",
                        "ds-scene-v1",
                        PlatformSerializationEndianness.LittleEndian))],
                outputRoot,
                Path.Combine(workingRoot, "tmp"),
                selectedBuildProfileId: "ds-default",
                selectedGraphicsProfileId: "ds-main-2d",
                selectedCodegenProfileId: "default",
                selectedBuildOptionValues: new Dictionary<string, string> {
                    ["startup-top-screen-color"] = "#FF0000",
                    ["startup-bottom-screen-color"] = "#0000FF"
                },
                selectedGraphicsOptionValues: new Dictionary<string, string>(),
                selectedCodegenOptionValues: new Dictionary<string, string>(),
                generatedCoreCppRootPath: generatedCoreRoot,
                selectedMediaProfileId: "ds-cartridge",
                selectedStorageProfileId: "nitrofs-package");

            PlatformBuildReport report = await builder.BuildAsync(
                request,
                new RecordingProgressReporter(),
                new RecordingDiagnosticReporter(),
                CancellationToken.None);

            Assert.True(report.Succeeded);
            Assert.Equal(sourceFontPath, platformCookSourceProcessor.LastFontAtlasSourceAssetPath);
        } finally {
            Directory.SetCurrentDirectory(previousCurrentDirectory);
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Builds one serialized scene asset for Nintendo DS builder tests.
    /// </summary>
    /// <param name="includeUnsupportedReturnToMenuComponent">Whether to include the unsupported city demo-disc return-to-menu component.</param>
    /// <returns>Serialized scene asset bytes.</returns>
    static byte[] BuildSceneAssetBytes(bool includeUnsupportedReturnToMenuComponent) {
        List<SceneComponentAssetRecord> components = new();
        if (includeUnsupportedReturnToMenuComponent) {
            components.Add(new SceneComponentAssetRecord {
                ComponentTypeId = "city.menu.DemoDiscReturnToMenuComponent, gameplay",
                ComponentIndex = 0,
                Payload = []
            });
        }

        components.Add(new SceneComponentAssetRecord {
            ComponentTypeId = "helengine.MeshComponent",
            ComponentIndex = components.Count,
            Payload = [1, 2, 3]
        });

        SceneAsset sceneAsset = new SceneAsset {
            Id = "scenes/rendering/cube_test.helen",
            RootEntities = [
                new SceneEntityAsset {
                    Id = 1,
                    Name = "CubeRoot",
                    LocalPosition = float3.Zero,
                    LocalScale = float3.One,
                    LocalOrientation = float4.Identity,
                    Components = components.ToArray(),
                    Children = Array.Empty<SceneEntityAsset>()
                }
            ]
        };
        return helengine.files.AssetSerializer.SerializeToBytes(sceneAsset);
    }

    /// <summary>
    /// Writes one minimal packaged font asset that exposes a source atlas texture payload for builder-owned atlas cook tests.
    /// </summary>
    /// <param name="fontAssetPath">Absolute packaged font path to write.</param>
    static void WriteTestFontAsset(string fontAssetPath) {
        if (string.IsNullOrWhiteSpace(fontAssetPath)) {
            throw new ArgumentException("Font asset path must be provided.", nameof(fontAssetPath));
        }

        string fontAssetDirectoryPath = Path.GetDirectoryName(fontAssetPath)
            ?? throw new InvalidOperationException("Unable to resolve the packaged font directory.");
        Directory.CreateDirectory(fontAssetDirectoryPath);

        FontAsset fontAsset = new(
            new FontInfo("Test Font", 16, 8f),
            null,
            new Dictionary<char, FontChar> {
                ['A'] = new FontChar(new float4(0f, 0f, 1f, 1f), 0f, 8f, 0f, 0f)
            },
            16f,
            8,
            8) {
            SourceTextureAsset = new TextureAsset {
                Id = "font-source",
                RuntimeAssetId = RuntimeAssetIdGenerator.Generate("font-source"),
                Width = 8,
                Height = 8,
                ColorFormat = TextureAssetColorFormat.Rgba32,
                AlphaPrecision = TextureAssetAlphaPrecision.A8,
                Colors = new byte[8 * 8 * 4]
            }
        };

        using FileStream stream = new(fontAssetPath, FileMode.Create, FileAccess.Write, FileShare.None);
        helengine.files.FontAssetBinarySerializer.Serialize(stream, fontAsset);
    }

    /// <summary>
    /// Writes one minimal packaged font asset that externalizes its cooked atlas texture path and omits raw atlas bytes.
    /// </summary>
    /// <param name="fontAssetPath">Absolute packaged font path to write.</param>
    /// <param name="cookedAtlasTextureRelativePath">Runtime-relative cooked atlas texture path persisted by the packaged font asset.</param>
    static void WriteExternalizedTestFontAsset(string fontAssetPath, string cookedAtlasTextureRelativePath) {
        if (string.IsNullOrWhiteSpace(fontAssetPath)) {
            throw new ArgumentException("Font asset path must be provided.", nameof(fontAssetPath));
        } else if (string.IsNullOrWhiteSpace(cookedAtlasTextureRelativePath)) {
            throw new ArgumentException("Cooked atlas texture relative path must be provided.", nameof(cookedAtlasTextureRelativePath));
        }

        string fontAssetDirectoryPath = Path.GetDirectoryName(fontAssetPath)
            ?? throw new InvalidOperationException("Unable to resolve the packaged font directory.");
        Directory.CreateDirectory(fontAssetDirectoryPath);

        FontAsset fontAsset = new(
            new FontInfo("Test Font", 16, 8f),
            null,
            new Dictionary<char, FontChar> {
                ['A'] = new FontChar(new float4(0f, 0f, 1f, 1f), 0f, 8f, 0f, 0f)
            },
            16f,
            8,
            8) {
            SourceTextureAsset = null,
            CookedAtlasTextureRelativePath = cookedAtlasTextureRelativePath
        };

        using FileStream stream = new(fontAssetPath, FileMode.Create, FileAccess.Write, FileShare.None);
        helengine.files.FontAssetBinarySerializer.Serialize(stream, fontAsset);
    }
}
