using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.ds.builder.tests.Builders;
using System.Runtime.Versioning;

namespace helengine.ds.builder.tests;

/// <summary>Exercises actual NSBTX cooking, native serialization and NitroFS staging for images and font atlases.</summary>
[SupportedOSPlatform("windows")]
public sealed class NintendoDsNativeTextureBuildTests {
    /// <summary>Checks that the actual builder preserves native encoding through validation and package staging.</summary>
    [Theory]
    [InlineData("texture")]
    [InlineData("font-atlas-texture")]
    public async Task Native_work_items_retain_gpu_storage_in_nitrofs(string kind) {
        string repository = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string working = Path.Combine(Path.GetDirectoryName(repository), "builds", "helengine-ds", "native-texture-tests", Guid.NewGuid().ToString("N"));
        string builderWorking = Path.Combine(working, "builder"), generated = Path.Combine(working, "generated-core");
        string package = NintendoDsBuildPathConventions.ResolvePackageSourceRootPath(builderWorking);
        Directory.CreateDirectory(Path.Combine(package, "cooked", "scenes"));
        WriteCodegenOrchestrationFixture(generated);
        SceneAsset sceneAsset = new SceneAsset { Id = "main", RootEntities = Array.Empty<SceneEntityAsset>() };
        File.WriteAllBytes(Path.Combine(package, "cooked", "scenes", "main.hasset"), helengine.files.AssetSerializer.SerializeToBytes(sceneAsset));
        string source = Path.Combine(AppContext.BaseDirectory, "Fixtures", "nsbtx", "a3i5.nsbtx");
        TextureAssetProcessorSettings settings = new TextureAssetProcessorSettings { MaxResolution = 8, ColorFormatId = "NintendoDs.A5I3", AlphaPrecision = TextureAssetAlphaPrecision.A5 };
        PlatformBuildScene scene = new PlatformBuildScene("main", "Main", "scene",
            new[] { new PlatformBuildPayloadReference("cooked/scenes/main.hasset", "cooked/scenes/main.hasset") },
            new[] { new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/main.hasset") });
        PlatformCookWorkItem item = new PlatformCookWorkItem("native-texture", source, kind, "ds", "runtime-texture", "cooked/imported/native.hasset",
            "native-ds", "source-hash", "settings-hash", NintendoDsTextureCookSettingsSerializer.Serialize(settings),
            new[] { new PlatformCookWorkItemMetadata("source-asset-id", "native-ds") });
        PlatformBuildManifest manifest = new PlatformBuildManifest(3, "project", "1.0.0", "1.0.0", "ds", "1", "main", new[] { scene },
            Array.Empty<PlatformBuildAsset>(), Array.Empty<PlatformBuildArtifact>(), Array.Empty<PlatformBuildCodeModule>(), Array.Empty<PlatformArtifactPlacement>(),
            new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()), new[] { item }, PlatformBuildRuntimeFeatureManifest.Empty);
        PlatformBuildRequest request = new PlatformBuildRequest(manifest,
            new[] { new PlatformBuildTargetVariant("release", "ds", "ds", "release") },
            new[] { new PlatformCookProfile("release", "DS Default", new PlatformCookProfileCapabilities("ds", "raw", "raw", "ds-scene-v1", PlatformSerializationEndianness.LittleEndian)) },
            Path.Combine(working, "out"), builderWorking,
            selectedBuildProfileId: "release", selectedGraphicsProfileId: "ds-main-2d", selectedCodegenProfileId: "default",
            selectedBuildOptionValues: new Dictionary<string, string> { ["startup-top-screen-color"] = "#FF0000", ["startup-bottom-screen-color"] = "#0000FF" },
            selectedGraphicsOptionValues: new Dictionary<string, string>(), selectedCodegenOptionValues: new Dictionary<string, string>(),
            generatedCoreCppRootPath: generated, selectedMediaProfileId: "ds-cartridge", selectedStorageProfileId: "nitrofs-package");
        FakeNintendoDsNativeBuildExecutor executor = new FakeNintendoDsNativeBuildExecutor();
        NintendoDsPlatformAssetBuilder builder = new NintendoDsPlatformAssetBuilder(executor, repository);
        var report = await builder.BuildAsync(request, new RecordingProgressReporter(), new RecordingDiagnosticReporter(), CancellationToken.None);
        Assert.True(report.Succeeded);
        string staged = Path.Combine(executor.Workspace.NitroFsRootPath, "cooked", "imported", "native.hasset");
        using TextureAsset cooked = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(staged)));
        Assert.Equal(kind == "texture" ? "native-ds" : "native-ds#atlas", cooked.Id);
        Assert.Equal(TextureAssetColorFormat.NintendoDsNative, cooked.ColorFormat);
        Assert.Equal(6, NintendoDsTextureCodec.ReadLayout(cooked).Format.Code);
        using TextureAsset expected = new NintendoDsPlatformCookSourceProcessor().CookTexture(source, "native-ds", settings);
        Assert.Equal(expected.Colors, cooked.Colors); Assert.Equal(expected.PaletteColors, cooked.PaletteColors);
    }
    /// <summary>Writes small declaration fixtures for builder orchestration; actual C++ generation and SDK compilation run separately.</summary>
    static void WriteCodegenOrchestrationFixture(string root) {
        Directory.CreateDirectory(Path.Combine(root, "runtime"));
        File.WriteAllText(Path.Combine(root, "helcpp_config.hpp"), "#pragma once");
        File.WriteAllText(Path.Combine(root, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
        File.WriteAllText(Path.Combine(root, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
        File.WriteAllText(Path.Combine(root, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
        File.WriteAllText(Path.Combine(root, "RuntimeComponentRegistry.cpp"), "#include \"RuntimeComponentRegistry.hpp\"\n#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
        File.WriteAllText(Path.Combine(root, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/main.hasset\"; }");
    }
}
