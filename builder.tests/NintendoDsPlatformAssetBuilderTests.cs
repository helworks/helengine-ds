using helengine.baseplatform.Definitions;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.ds.builder.tests.Builders;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS builder metadata exposed to the editor.
/// </summary>
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
        string packageRoot = Path.Combine(workingRoot, "package");
        string previousCurrentDirectory = Directory.GetCurrentDirectory();

        try {
            Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
            Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(generatedCoreRoot, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(
                Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(Path.Combine(packageRoot, "cooked", "scenes", "startup.hasset"), "scene");

            Directory.SetCurrentDirectory(packageRoot);

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
}
