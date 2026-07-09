using System.Text.RegularExpressions;
using helengine;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using System.Runtime.Versioning;

namespace helengine.ds.builder;

/// <summary>
/// Executes builder-owned smoke and native verification flows for Nintendo DS builds.
/// </summary>
[SupportedOSPlatform("windows")]
internal static class NintendoDsVerificationHarness {
    /// <summary>
    /// Stores the startup-scene manifest function marker used by generated core.
    /// </summary>
    static readonly Regex StartupScenePathRegex = new(
        "(?:kRuntimeStartupSceneRelativePath\\[\\]\\s*=|return)\\s*\\\"(?<path>[^\\\"]+)\\\"",
        RegexOptions.CultureInvariant | RegexOptions.Compiled);

    /// <summary>
    /// Executes one builder-owned smoke build using fresh staged inputs and a fake native executor.
    /// </summary>
    public static void RunSmokeTest() {
        string repositoryRootPath = ResolveRepositoryRootPath();
        string workingRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-builder-smoke-" + Guid.NewGuid().ToString("N"));
        string generatedCoreRootPath = Path.Combine(workingRootPath, "generated-core");
        string stagingRootPath = NintendoDsBuildPathConventions.ResolvePackageSourceRootPath(Path.Combine(workingRootPath, "tmp"));
        string outputRootPath = Path.Combine(workingRootPath, "out");
        string sceneSourcePath = Path.Combine(stagingRootPath, "cooked", "scenes", "startup.hasset");

        try {
            PrepareGeneratedCoreFixture(generatedCoreRootPath, "cooked/scenes/startup.hasset");
            Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath)
                ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS smoke-test scene directory."));
            File.WriteAllBytes(sceneSourcePath, BuildSceneAssetBytes());

            NintendoDsSmokeTestNativeBuildExecutor nativeBuildExecutor = new();
            NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRootPath);
            PlatformBuildRequest request = CreateBuildRequest(
                outputRootPath,
                Path.Combine(workingRootPath, "tmp"),
                generatedCoreRootPath,
                "cooked/scenes/startup.hasset",
                EnumeratePayloadRelativePaths(stagingRootPath));
            PlatformBuildReport report = builder.BuildAsync(
                request,
                new NintendoDsNullProgressReporter(),
                new NintendoDsNullDiagnosticReporter(),
                CancellationToken.None).GetAwaiter().GetResult();

            if (!report.Succeeded) {
                throw new InvalidOperationException("Smoke test build failed.");
            } else if (nativeBuildExecutor.Workspace == null) {
                throw new InvalidOperationException("Smoke test native executor did not receive a build workspace.");
            } else if (!File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "runtime", "ds_startup_manifest.bin"))) {
                throw new InvalidOperationException("Smoke test startup manifest is missing from staged NitroFS.");
            } else if (!File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "cooked", "scenes", "startup.hasset"))) {
                throw new InvalidOperationException("Smoke test scene payload is missing from staged NitroFS.");
            } else if (!File.Exists(Path.Combine(nativeBuildExecutor.Workspace.StagedGeneratedCoreRootPath, "helengine_core_amalgamated.cpp"))) {
                throw new InvalidOperationException("Smoke test generated core was not staged into the DS workspace.");
            } else if (!File.Exists(nativeBuildExecutor.Workspace.ExportPackagePath)) {
                throw new InvalidOperationException("Smoke test exported package is missing.");
            }

            Console.WriteLine("Smoke test passed.");
        } finally {
            TryDeleteDirectory(workingRootPath);
        }
    }

    /// <summary>
    /// Runs smoke-first native verification using editor-produced generated core and a staged package root.
    /// </summary>
    /// <param name="generatedCoreRootPath">Editor-produced generated-core root.</param>
    /// <param name="stagingRootPath">Editor-produced staged package root containing cooked files.</param>
    /// <param name="outputRootPath">Output root that should receive the exported package.</param>
    public static void RunNativeVerification(string generatedCoreRootPath, string stagingRootPath, string outputRootPath) {
        RunNativeVerification(
            generatedCoreRootPath,
            stagingRootPath,
            outputRootPath,
            new NintendoDsNativeBuildExecutor());
    }

    /// <summary>
    /// Runs smoke-first native verification using one explicit native-build executor.
    /// </summary>
    /// <param name="generatedCoreRootPath">Editor-produced generated-core root.</param>
    /// <param name="stagingRootPath">Editor-produced staged package root containing cooked files.</param>
    /// <param name="outputRootPath">Output root that should receive the exported package.</param>
    /// <param name="nativeBuildExecutor">Native-build executor used for the final DS build.</param>
    internal static void RunNativeVerification(
        string generatedCoreRootPath,
        string stagingRootPath,
        string outputRootPath,
        INintendoDsNativeBuildExecutor nativeBuildExecutor) {
        if (string.IsNullOrWhiteSpace(generatedCoreRootPath)) {
            throw new ArgumentException("Generated-core root path must be provided.", nameof(generatedCoreRootPath));
        } else if (string.IsNullOrWhiteSpace(stagingRootPath)) {
            throw new ArgumentException("Staging root path must be provided.", nameof(stagingRootPath));
        } else if (string.IsNullOrWhiteSpace(outputRootPath)) {
            throw new ArgumentException("Output root path must be provided.", nameof(outputRootPath));
        } else if (nativeBuildExecutor == null) {
            throw new ArgumentNullException(nameof(nativeBuildExecutor));
        }

        RunSmokeTest();

        string fullGeneratedCoreRootPath = Path.GetFullPath(generatedCoreRootPath);
        string fullStagingRootPath = Path.GetFullPath(stagingRootPath);
        string fullOutputRootPath = Path.GetFullPath(outputRootPath);
        string repositoryRootPath = ResolveRepositoryRootPath();
        string workingRootPath = Path.Combine(fullOutputRootPath, "_builder");
        string packageSourceRootPath = NintendoDsBuildPathConventions.ResolvePackageSourceRootPath(workingRootPath);
        string startupSceneRelativePath = ReadStartupSceneRelativePath(fullGeneratedCoreRootPath);
        string[] payloadRelativePaths = EnumeratePayloadRelativePaths(fullStagingRootPath);

        StagePackageSourceRoot(fullStagingRootPath, packageSourceRootPath);
        NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRootPath);
        PlatformBuildRequest request = CreateBuildRequest(
            fullOutputRootPath,
            workingRootPath,
            fullGeneratedCoreRootPath,
            startupSceneRelativePath,
            payloadRelativePaths);
        PlatformBuildReport report = builder.BuildAsync(
            request,
            new NintendoDsNullProgressReporter(),
            new NintendoDsNullDiagnosticReporter(),
            CancellationToken.None).GetAwaiter().GetResult();

        if (!report.Succeeded) {
            throw new InvalidOperationException("Native verification build failed.");
        }

        string exportedPackagePath = Path.Combine(fullOutputRootPath, "helengine_ds.nds");
        if (!File.Exists(exportedPackagePath)) {
            throw new InvalidOperationException("Native verification package was not produced.");
        }

        Console.WriteLine("Native verification passed.");
        Console.WriteLine(exportedPackagePath);
    }

    /// <summary>
    /// Creates the generated-core fixture required by the DS smoke run.
    /// </summary>
    /// <param name="generatedCoreRootPath">Generated-core root path to populate.</param>
    /// <param name="startupSceneRelativePath">Startup-scene relative path encoded into the runtime manifest source.</param>
    static void PrepareGeneratedCoreFixture(string generatedCoreRootPath, string startupSceneRelativePath) {
        if (string.IsNullOrWhiteSpace(generatedCoreRootPath)) {
            throw new ArgumentException("Generated-core root path must be provided.", nameof(generatedCoreRootPath));
        } else if (string.IsNullOrWhiteSpace(startupSceneRelativePath)) {
            throw new ArgumentException("Startup-scene relative path must be provided.", nameof(startupSceneRelativePath));
        }

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
    /// Creates one DS build request for smoke or native verification runs.
    /// </summary>
    /// <param name="outputRootPath">Output root that should receive the exported package.</param>
    /// <param name="workingRootPath">Builder-owned working root used for staged DS inputs.</param>
    /// <param name="generatedCoreRootPath">Editor-produced generated-core root consumed by the builder.</param>
    /// <param name="startupSceneRelativePath">Startup-scene payload path staged under the package root.</param>
    /// <param name="payloadRelativePaths">All staged payload paths that should be copied into NitroFS.</param>
    /// <returns>Prepared build request.</returns>
    static PlatformBuildRequest CreateBuildRequest(
        string outputRootPath,
        string workingRootPath,
        string generatedCoreRootPath,
        string startupSceneRelativePath,
        IReadOnlyList<string> payloadRelativePaths) {
        if (string.IsNullOrWhiteSpace(startupSceneRelativePath)) {
            throw new ArgumentException("Startup-scene relative path must be provided.", nameof(startupSceneRelativePath));
        } else if (payloadRelativePaths == null || payloadRelativePaths.Count == 0) {
            throw new InvalidOperationException("At least one staged payload path must be provided.");
        }

        List<PlatformBuildPayloadReference> startupPayloadReferences = [];
        List<PlatformBuildAsset> looseAssets = [];
        for (int index = 0; index < payloadRelativePaths.Count; index++) {
            string payloadRelativePath = payloadRelativePaths[index];
            PlatformBuildPayloadReference payloadReference = new(payloadRelativePath, payloadRelativePath);
            if (string.Equals(payloadRelativePath, startupSceneRelativePath, StringComparison.OrdinalIgnoreCase)) {
                startupPayloadReferences.Add(payloadReference);
                continue;
            }

            looseAssets.Add(new PlatformBuildAsset(
                payloadRelativePath,
                Path.GetFileNameWithoutExtension(payloadRelativePath),
                payloadRelativePath,
                payloadReference,
                []));
        }

        if (startupPayloadReferences.Count == 0) {
            throw new InvalidOperationException($"Startup scene payload '{startupSceneRelativePath}' was not found under the staged package root.");
        }

        PlatformBuildScene[] scenes = [
            new PlatformBuildScene(
                NintendoDsStartupSceneIds.GeneratedBootSceneId,
                "Startup",
                "scene",
                [.. startupPayloadReferences],
                [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, startupSceneRelativePath)])
        ];
        PlatformBuildManifest manifest = new(
            3,
            "project",
            "1.0.0",
            "1.0.0",
            "ds",
            "1",
            "startup",
            scenes,
            [.. looseAssets],
            Array.Empty<PlatformBuildArtifact>(),
            Array.Empty<PlatformBuildCodeModule>(),
            Array.Empty<PlatformArtifactPlacement>(),
            new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

        return new PlatformBuildRequest(
            manifest,
            [new PlatformBuildTargetVariant("release", "ds", "ds", "release")],
            [new PlatformCookProfile(
                "release",
                "DS Default",
                new PlatformCookProfileCapabilities(
                    "ds",
                    "raw",
                    "raw",
                    "ds-scene-v1",
                    PlatformSerializationEndianness.LittleEndian))],
            outputRootPath,
            workingRootPath,
            selectedBuildProfileId: "release",
            selectedGraphicsProfileId: "ds-main-2d",
            selectedCodegenProfileId: "default",
            selectedBuildOptionValues: new Dictionary<string, string> {
                ["startup-top-screen-color"] = "#FF0000",
                ["startup-bottom-screen-color"] = "#0000FF"
            },
            selectedGraphicsOptionValues: new Dictionary<string, string>(),
            selectedCodegenOptionValues: new Dictionary<string, string>(),
            generatedCoreCppRootPath: generatedCoreRootPath,
            selectedMediaProfileId: "ds-cartridge",
            selectedStorageProfileId: "nitrofs-package");
    }

    /// <summary>
    /// Enumerates all staged payload files under the package root as forward-slash relative paths.
    /// </summary>
    /// <param name="stagingRootPath">Staged package root to inspect.</param>
    /// <returns>Sorted forward-slash relative payload paths.</returns>
    static string[] EnumeratePayloadRelativePaths(string stagingRootPath) {
        if (string.IsNullOrWhiteSpace(stagingRootPath)) {
            throw new ArgumentException("Staging root path must be provided.", nameof(stagingRootPath));
        }

        string fullStagingRootPath = Path.GetFullPath(stagingRootPath);
        if (!Directory.Exists(fullStagingRootPath)) {
            throw new DirectoryNotFoundException($"Staging root '{fullStagingRootPath}' was not found.");
        }

        string[] filePaths = Directory.GetFiles(fullStagingRootPath, "*", SearchOption.AllDirectories);
        string[] relativePaths = new string[filePaths.Length];
        for (int index = 0; index < filePaths.Length; index++) {
            relativePaths[index] = Path.GetRelativePath(fullStagingRootPath, filePaths[index]).Replace('\\', '/');
        }

        Array.Sort(relativePaths, StringComparer.OrdinalIgnoreCase);
        return relativePaths;
    }

    /// <summary>
    /// Copies one external staged package root into the builder-owned working-root package source.
    /// </summary>
    /// <param name="sourceRootPath">External staged package root produced by the editor.</param>
    /// <param name="destinationRootPath">Builder-owned package source root under the working root.</param>
    static void StagePackageSourceRoot(string sourceRootPath, string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(sourceRootPath)) {
            throw new ArgumentException("Source root path must be provided.", nameof(sourceRootPath));
        } else if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Destination root path must be provided.", nameof(destinationRootPath));
        }

        string fullSourceRootPath = Path.GetFullPath(sourceRootPath);
        string fullDestinationRootPath = Path.GetFullPath(destinationRootPath);
        if (!Directory.Exists(fullSourceRootPath)) {
            throw new DirectoryNotFoundException($"Staging root '{fullSourceRootPath}' was not found.");
        }

        if (Directory.Exists(fullDestinationRootPath)) {
            Directory.Delete(fullDestinationRootPath, recursive: true);
        }

        CopyDirectory(fullSourceRootPath, fullDestinationRootPath);
    }

    /// <summary>
    /// Recursively copies one directory tree into the destination path.
    /// </summary>
    /// <param name="sourceDirectoryPath">Existing directory to copy.</param>
    /// <param name="destinationDirectoryPath">Destination directory to populate.</param>
    static void CopyDirectory(string sourceDirectoryPath, string destinationDirectoryPath) {
        Directory.CreateDirectory(destinationDirectoryPath);

        string[] filePaths = Directory.GetFiles(sourceDirectoryPath);
        for (int index = 0; index < filePaths.Length; index++) {
            string filePath = filePaths[index];
            string fileName = Path.GetFileName(filePath);
            string destinationFilePath = Path.Combine(destinationDirectoryPath, fileName);
            File.Copy(filePath, destinationFilePath, overwrite: true);
        }

        string[] directoryPaths = Directory.GetDirectories(sourceDirectoryPath);
        for (int index = 0; index < directoryPaths.Length; index++) {
            string directoryPath = directoryPaths[index];
            string directoryName = Path.GetFileName(directoryPath);
            string destinationChildDirectoryPath = Path.Combine(destinationDirectoryPath, directoryName);
            CopyDirectory(directoryPath, destinationChildDirectoryPath);
        }
    }

    /// <summary>
    /// Reads the startup-scene payload path encoded into generated core.
    /// </summary>
    /// <param name="generatedCoreRootPath">Generated-core root to inspect.</param>
    /// <returns>Forward-slash startup-scene relative path.</returns>
    static string ReadStartupSceneRelativePath(string generatedCoreRootPath) {
        if (string.IsNullOrWhiteSpace(generatedCoreRootPath)) {
            throw new ArgumentException("Generated-core root path must be provided.", nameof(generatedCoreRootPath));
        }

        string manifestSourcePath = Path.Combine(generatedCoreRootPath, "runtime", "runtime_startup_manifest.cpp");
        if (!File.Exists(manifestSourcePath)) {
            throw new FileNotFoundException("Generated core root does not contain runtime/runtime_startup_manifest.cpp.", manifestSourcePath);
        }

        string manifestSource = File.ReadAllText(manifestSourcePath);
        Match match = StartupScenePathRegex.Match(manifestSource);
        if (!match.Success) {
            throw new InvalidOperationException("Unable to resolve the startup scene relative path from runtime_startup_manifest.cpp.");
        }

        return match.Groups["path"].Value.Replace('\\', '/');
    }

    /// <summary>
    /// Builds one serialized startup-scene payload for the DS smoke run.
    /// </summary>
    /// <returns>Serialized scene-asset bytes.</returns>
    static byte[] BuildSceneAssetBytes() {
        SceneAsset sceneAsset = new SceneAsset {
            Id = "scenes/startup.helen",
            RootEntities = [
                new SceneEntityAsset {
                    Id = 1,
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

    /// <summary>
    /// Resolves the DS repository root from an environment override or from the builder assembly location.
    /// </summary>
    /// <returns>Absolute DS repository root path.</returns>
    static string ResolveRepositoryRootPath() {
        string configuredRepositoryRootPath = Environment.GetEnvironmentVariable("HELENGINE_DS_REPOSITORY_ROOT") ?? string.Empty;
        if (!string.IsNullOrWhiteSpace(configuredRepositoryRootPath) && File.Exists(Path.Combine(configuredRepositoryRootPath, "Makefile"))) {
            return Path.GetFullPath(configuredRepositoryRootPath);
        }

        string assemblyLocation = typeof(NintendoDsVerificationHarness).Assembly.Location;
        if (string.IsNullOrWhiteSpace(assemblyLocation)) {
            throw new InvalidOperationException("Unable to resolve the Nintendo DS builder assembly location.");
        }

        string baseDirectory = Path.GetDirectoryName(assemblyLocation)
            ?? throw new InvalidOperationException($"Unable to resolve the Nintendo DS builder base directory from '{assemblyLocation}'.");
        string repositoryRootPath = Path.GetFullPath(Path.Combine(baseDirectory, "..", "..", "..", ".."));
        if (!File.Exists(Path.Combine(repositoryRootPath, "Makefile"))) {
            throw new InvalidOperationException("Unable to resolve the helengine-ds repository root for builder verification.");
        }

        return repositoryRootPath;
    }

    /// <summary>
    /// Deletes one directory tree when it exists and the caller is done with it.
    /// </summary>
    /// <param name="path">Directory path to delete.</param>
    static void TryDeleteDirectory(string path) {
        try {
            if (!string.IsNullOrWhiteSpace(path) && Directory.Exists(path)) {
                Directory.Delete(path, recursive: true);
            }
        } catch {
        }
    }
}

