using helengine;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.ds.builder.tests.Builders;
using helengine.files;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS builder metadata exposed to the editor.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
public class NintendoDsPlatformAssetBuilderTests {
    /// <summary>
    /// Verifies the builder descriptor and platform definition expose the expected Nintendo DS metadata.
    /// </summary>
    [Fact]
    public void Descriptor_and_definition_expose_ds_metadata() {
        NintendoDsPlatformAssetBuilder builder = new();

        Assert.Equal("helengine.ds.builder", builder.Descriptor.BuilderId);
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
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "startup.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "startup",
                    "Startup",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/startup.hasset", "cooked/scenes/startup.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/startup.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "startup",
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
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "startup.hasset")));
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
    public async Task BuildAsync_whenStagedSceneContainsUnsupportedReturnToMenuComponent_sanitizesSceneAsset() {
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
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/rendering/cube_test.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "rendering", "cube_test.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: true));

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
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/rendering/cube_test.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
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

            Assert.DoesNotContain(
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
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/startup.hasset\"; }");

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "startup",
                    "Startup",
                    "scene",
                    Array.Empty<PlatformBuildPayloadReference>(),
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/startup.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "startup",
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
                "Nintendo DS startup scene 'startup' did not stage cooked payload 'cooked/scenes/startup.hasset' into NitroFS.",
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
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/startup.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "startup.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "startup",
                    "Startup",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/startup.hasset", "cooked/scenes/startup.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/startup.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "startup",
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
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/startup.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "startup.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.CreateDirectory(outputRoot);
            Directory.SetCurrentDirectory(outputRoot);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "startup",
                    "Startup",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/startup.hasset", "cooked/scenes/startup.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/startup.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "startup",
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
                Path.Combine(packageRoot, "cooked", "scenes", "startup.hasset"),
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
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/startup.hasset\"; }");
            File.WriteAllBytes(
                Path.Combine(packageRoot, "cooked", "scenes", "startup.hasset"),
                BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

            Directory.SetCurrentDirectory(unrelatedCurrentDirectory);

            RecordingDiagnosticReporter diagnosticReporter = new();
            RecordingProgressReporter progressReporter = new();
            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

            PlatformBuildScene[] scenes = [
                new PlatformBuildScene(
                    "startup",
                    "Startup",
                    "scene",
                    [new PlatformBuildPayloadReference("cooked/scenes/startup.hasset", "cooked/scenes/startup.hasset")],
                    [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/startup.hasset")])
            ];
            PlatformBuildManifest manifest = new(
                3,
                "project",
                "1.0.0",
                "1.0.0",
                "startup",
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
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "startup.hasset")));
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
                    Id = "cube-root",
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
}
