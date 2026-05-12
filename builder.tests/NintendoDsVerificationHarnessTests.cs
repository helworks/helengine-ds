using helengine.ds.builder.tests.Builders;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the builder-owned Nintendo DS verification harness flows.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
public class NintendoDsVerificationHarnessTests {
    /// <summary>
    /// Verifies native verification runs the smoke harness first and then packages the editor-produced inputs through the builder workspace.
    /// </summary>
    [Fact]
    public void RunNativeVerification_whenInputsAreValid_runs_smoke_then_builder_owned_native_flow() {
        TextWriter previousOutput = Console.Out;
        StringWriter output = new StringWriter();
        string safeDirectoryPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string workingRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-verification-" + Guid.NewGuid().ToString("N"));
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string stagingRootPath = Path.Combine(workingRootPath, "staging");
        string outputRootPath = Path.Combine(workingRootPath, "out");

        try {
            Directory.SetCurrentDirectory(safeDirectoryPath);
            PrepareGeneratedCoreFixture(generatedCoreRootPath, "cooked/scenes/startup.hasset");
            Directory.CreateDirectory(Path.Combine(stagingRootPath, "cooked", "scenes"));
            File.WriteAllBytes(
                Path.Combine(stagingRootPath, "cooked", "scenes", "startup.hasset"),
                BuildSceneAssetBytes());
            File.WriteAllBytes(
                Path.Combine(stagingRootPath, "cooked", "shared_texture.bin"),
                [1, 2, 3, 4]);

            FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
            Console.SetOut(output);

            NintendoDsVerificationHarness.RunNativeVerification(
                generatedCoreRootPath,
                stagingRootPath,
                outputRootPath,
                nativeBuildExecutor);

            Assert.Contains("Smoke test passed.", output.ToString(), StringComparison.Ordinal);
            Assert.Contains("Native verification passed.", output.ToString(), StringComparison.Ordinal);
            Assert.NotNull(nativeBuildExecutor.Workspace);
            Assert.Equal(Path.Combine(outputRootPath, "_builder"), nativeBuildExecutor.Workspace.WorkingRootPath);
            Assert.Equal(Path.GetFullPath(generatedCoreRootPath), nativeBuildExecutor.Workspace.GeneratedCoreRootPath);
            Assert.True(File.Exists(nativeBuildExecutor.Workspace.ExportPackagePath));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "runtime", "ds_startup_manifest.bin")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "startup.hasset")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "shared_texture.bin")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.StagedGeneratedCoreRootPath, "helengine_core_amalgamated.cpp")));
            Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.WorkingRootPath, NintendoDsBuildPathConventions.PackageSourceDirectoryName, "cooked", "scenes", "startup.hasset")));
        } finally {
            Console.SetOut(previousOutput);
            Directory.SetCurrentDirectory(safeDirectoryPath);

            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }

            output.Dispose();
        }
    }

    /// <summary>
    /// Creates the generated-core fixture required by the verification harness test.
    /// </summary>
    /// <param name="generatedCoreRootPath">Generated-core root path to populate.</param>
    /// <param name="startupSceneRelativePath">Startup-scene relative path encoded into generated core.</param>
    static void PrepareGeneratedCoreFixture(string generatedCoreRootPath, string startupSceneRelativePath) {
        string runtimeRootPath = Path.Combine(generatedCoreRootPath, "runtime");
        Directory.CreateDirectory(runtimeRootPath);
        File.WriteAllText(Path.Combine(generatedCoreRootPath, "helcpp_config.hpp"), "#pragma once");
        File.WriteAllText(Path.Combine(generatedCoreRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
        File.WriteAllText(Path.Combine(generatedCoreRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
        File.WriteAllText(
            Path.Combine(generatedCoreRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"),
            "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
        File.WriteAllText(
            Path.Combine(generatedCoreRootPath, "RuntimeComponentRegistry.cpp"),
            "#include \"RuntimeComponentRegistry.hpp\"\n"
            + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
            + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
        File.WriteAllText(
            Path.Combine(runtimeRootPath, "runtime_startup_manifest.cpp"),
            $"const char* he_get_runtime_startup_scene_relative_path() {{ return \"{startupSceneRelativePath}\"; }}");
    }

    /// <summary>
    /// Builds one serialized scene asset for the verification harness test.
    /// </summary>
    /// <returns>Serialized scene-asset bytes.</returns>
    static byte[] BuildSceneAssetBytes() {
        SceneAsset sceneAsset = new SceneAsset {
            Id = "scenes/startup.helen",
            RootEntities = [
                new SceneEntityAsset {
                    Id = "startup-root",
                    Name = "StartupRoot",
                    LocalPosition = float3.Zero,
                    LocalScale = float3.One,
                    LocalOrientation = float4.Identity,
                    Components = [
                        new SceneComponentAssetRecord {
                            ComponentTypeId = "helengine.MeshComponent",
                            ComponentIndex = 0,
                            Payload = [1, 2, 3]
                        }
                    ],
                    Children = Array.Empty<SceneEntityAsset>()
                }
            ]
        };
        return helengine.files.AssetSerializer.SerializeToBytes(sceneAsset);
    }
}
