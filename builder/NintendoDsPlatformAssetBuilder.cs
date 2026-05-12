using helengine.baseplatform.Builders;
using helengine.baseplatform.Descriptors;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Results;
using helengine.baseplatform.Targets;

namespace helengine.ds.builder;

/// <summary>
/// Implements the Nintendo DS platform asset builder contract.
/// </summary>
public sealed class NintendoDsPlatformAssetBuilder : IPlatformAssetBuilder {
    /// <summary>
    /// Environment variable that can override the Nintendo DS repository root.
    /// </summary>
    const string RepositoryRootEnvironmentVariableName = "HELENGINE_DS_REPOSITORY_ROOT";

    /// <summary>
    /// Stores the native build executor seam used by later build-orchestration tasks.
    /// </summary>
    readonly INintendoDsNativeBuildExecutor NativeBuildExecutor;

    /// <summary>
    /// Stores the resolved Nintendo DS repository root used for native packaging.
    /// </summary>
    readonly string RepositoryRootPath;

    /// <summary>
    /// Stores the builder-owned startup-manifest writer used to stage NitroFS payloads.
    /// </summary>
    readonly NintendoDsStartupManifestWriter StartupManifestWriter;

    /// <summary>
    /// Stores the generated-core stager used to copy runtime sources into the Docker-visible workspace.
    /// </summary>
    readonly NintendoDsGeneratedCoreStager GeneratedCoreStager;

    /// <summary>
    /// Stores the NitroFS asset stager used to mirror cooked payload files into the staged runtime tree.
    /// </summary>
    readonly NintendoDsNitroFsAssetStager NitroFsAssetStager;

    /// <summary>
    /// Stores the staged scene-asset sanitizer used to strip unsupported runtime-only components before native packaging.
    /// </summary>
    readonly NintendoDsSceneAssetSanitizer SceneAssetSanitizer;

    /// <summary>
    /// Initializes one Nintendo DS builder with the default native build executor.
    /// </summary>
    public NintendoDsPlatformAssetBuilder() {
        NativeBuildExecutor = new NintendoDsNativeBuildExecutor();
        RepositoryRootPath = string.Empty;
        StartupManifestWriter = new NintendoDsStartupManifestWriter();
        GeneratedCoreStager = new NintendoDsGeneratedCoreStager();
        NitroFsAssetStager = new NintendoDsNitroFsAssetStager();
        SceneAssetSanitizer = new NintendoDsSceneAssetSanitizer();
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    /// <summary>
    /// Initializes one Nintendo DS builder with a custom native build executor for tests.
    /// </summary>
    /// <param name="nativeBuildExecutor">Native build executor override used by tests.</param>
    /// <param name="repositoryRootPath">Repository root override used by tests.</param>
    internal NintendoDsPlatformAssetBuilder(INintendoDsNativeBuildExecutor nativeBuildExecutor, string repositoryRootPath) {
        NativeBuildExecutor = nativeBuildExecutor ?? throw new ArgumentNullException(nameof(nativeBuildExecutor));
        RepositoryRootPath = string.IsNullOrWhiteSpace(repositoryRootPath)
            ? throw new ArgumentException("Repository root path must be provided.", nameof(repositoryRootPath))
            : Path.GetFullPath(repositoryRootPath);
        StartupManifestWriter = new NintendoDsStartupManifestWriter();
        GeneratedCoreStager = new NintendoDsGeneratedCoreStager();
        NitroFsAssetStager = new NintendoDsNitroFsAssetStager();
        SceneAssetSanitizer = new NintendoDsSceneAssetSanitizer();
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    /// <summary>
    /// Gets the explicit builder descriptor exposed to the editor.
    /// </summary>
    public PlatformBuilderDescriptor Descriptor { get; }

    /// <summary>
    /// Gets the typed Nintendo DS platform definition exposed to the editor.
    /// </summary>
    public PlatformDefinition Definition { get; }

    /// <summary>
    /// Material cooking is not part of the startup-manifest vertical slice.
    /// </summary>
    /// <param name="request">Material cook request that is not supported in this slice.</param>
    /// <returns>No value because the operation is not supported.</returns>
    public PlatformMaterialCookResult CookMaterial(PlatformMaterialCookRequest request) {
        throw new NotSupportedException("Nintendo DS material cooking is not part of the startup-manifest slice.");
    }

    /// <summary>
    /// Build orchestration is implemented in a later task of the startup-manifest plan.
    /// </summary>
    /// <param name="request">Build request to process.</param>
    /// <param name="progressReporter">Progress reporter that receives streamed updates.</param>
    /// <param name="diagnosticReporter">Diagnostic reporter that receives streamed diagnostics.</param>
    /// <param name="cancellationToken">Cancellation token used to stop the build cooperatively.</param>
    /// <returns>Final Nintendo DS build report for the supplied request.</returns>
    public async Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        } else if (progressReporter == null) {
            throw new ArgumentNullException(nameof(progressReporter));
        } else if (diagnosticReporter == null) {
            throw new ArgumentNullException(nameof(diagnosticReporter));
        } else if (string.IsNullOrWhiteSpace(request.GeneratedCoreCppRootPath)) {
            throw new ArgumentException("Generated core root path must be provided for Nintendo DS builds.", nameof(request));
        }

        string topScreenColor = ReadRequiredBuildOption(request.SelectedBuildOptionValues, "startup-top-screen-color");
        string bottomScreenColor = ReadRequiredBuildOption(request.SelectedBuildOptionValues, "startup-bottom-screen-color");
        string repositoryRootPath = string.IsNullOrWhiteSpace(RepositoryRootPath)
            ? ResolveRepositoryRootPath()
            : RepositoryRootPath;

        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);

        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRootPath,
            request.WorkingRoot,
            request.OutputRoot,
            request.GeneratedCoreCppRootPath);
        string packageSourceRootPath = NintendoDsBuildPathConventions.ResolvePackageSourceRootPath(request.WorkingRoot);
        ValidatePackageSourceRootPath(packageSourceRootPath);

        ResetDirectory(workspace.NitroFsRootPath);
        StartupManifestWriter.Write(workspace.NitroFsRootPath, topScreenColor, bottomScreenColor);
        NitroFsAssetStager.Stage(request.Manifest, packageSourceRootPath, workspace.NitroFsRootPath);
        SceneAssetSanitizer.SanitizeStagedSceneAssets(workspace.NitroFsRootPath);
        GeneratedCoreStager.Stage(workspace.GeneratedCoreRootPath, workspace.StagedGeneratedCoreRootPath);
        ValidateStartupScenePayloadStaged(request.Manifest, workspace.NitroFsRootPath);
        progressReporter.Report(new PlatformBuildProgressUpdate(
            "Write Startup Manifest",
            "runtime/ds_startup_manifest.bin",
            1,
            2,
            "Wrote the DS startup manifest into the staged NitroFS root."));

        NativeBuildExecutor.Build(workspace, cancellationToken);
        progressReporter.Report(new PlatformBuildProgressUpdate(
            "Build Nintendo DS Package",
            "helengine_ds.nds",
            2,
            2,
            "Built the Nintendo DS package."));

        PlatformBuildReport report = new(
            File.Exists(workspace.ExportPackagePath),
            [],
            BuildSceneOutcomes(request.Manifest.Scenes),
            BuildLooseAssetOutcomes(request.Manifest.LooseAssets));
        return await Task.FromResult(report);
    }

    /// <summary>
    /// Creates the standard builder descriptor shared by both constructors.
    /// </summary>
    /// <returns>The Nintendo DS builder descriptor.</returns>
    static PlatformBuilderDescriptor CreateDescriptor() {
        return new PlatformBuilderDescriptor(
            "helengine.ds.builder",
            "1.0.0",
            "ds",
            new EngineCompatibilityRange("1.0.0", "999.0.0"),
            new ManifestCompatibilityRange(1, 3),
            ["ds"],
            ["ds-default"]);
    }

    /// <summary>
    /// Verifies the selected startup scene staged its cooked payload into NitroFS before native packaging starts.
    /// </summary>
    /// <param name="manifest">Resolved build manifest that identifies the startup scene.</param>
    /// <param name="nitroFsRootPath">NitroFS root that should already contain the staged startup-scene payload.</param>
    static void ValidateStartupScenePayloadStaged(PlatformBuildManifest manifest, string nitroFsRootPath) {
        if (manifest == null) {
            throw new ArgumentNullException(nameof(manifest));
        } else if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS root path must be provided.", nameof(nitroFsRootPath));
        } else if (string.IsNullOrWhiteSpace(manifest.StartupSceneId)) {
            return;
        }

        PlatformBuildScene startupScene = FindStartupScene(manifest);
        string startupSceneRelativePath = ReadStartupSceneRelativePath(startupScene);
        string stagedStartupScenePath = Path.Combine(
            nitroFsRootPath,
            startupSceneRelativePath.Replace('/', Path.DirectorySeparatorChar));
        if (File.Exists(stagedStartupScenePath)) {
            return;
        }

        throw new InvalidOperationException(
            $"Nintendo DS startup scene '{startupScene.SceneId}' did not stage cooked payload '{startupSceneRelativePath}' into NitroFS.");
    }

    /// <summary>
    /// Reads one required Nintendo DS build option value.
    /// </summary>
    /// <param name="values">Selected build option values keyed by option id.</param>
    /// <param name="key">Build option id to resolve.</param>
    /// <returns>Resolved non-empty build option value.</returns>
    static string ReadRequiredBuildOption(IReadOnlyDictionary<string, string> values, string key) {
        if (values == null) {
            throw new ArgumentNullException(nameof(values));
        } else if (string.IsNullOrWhiteSpace(key)) {
            throw new ArgumentException("Build option id must be provided.", nameof(key));
        }

        if (!values.TryGetValue(key, out string value) || string.IsNullOrWhiteSpace(value)) {
            throw new InvalidOperationException($"Missing required Nintendo DS build option '{key}'.");
        }

        return value;
    }

    /// <summary>
    /// Verifies the builder-owned staged package source exists before NitroFS staging begins.
    /// </summary>
    /// <param name="packageSourceRootPath">Builder-owned staged package source root.</param>
    static void ValidatePackageSourceRootPath(string packageSourceRootPath) {
        if (string.IsNullOrWhiteSpace(packageSourceRootPath)) {
            throw new ArgumentException("Package source root path must be provided.", nameof(packageSourceRootPath));
        } else if (!Directory.Exists(packageSourceRootPath)) {
            throw new InvalidOperationException(
                $"Nintendo DS builds require staged package content under '{NintendoDsBuildPathConventions.PackageSourceDirectoryName}' inside the working root.");
        }
    }

    /// <summary>
    /// Finds the manifest scene selected as the runtime startup scene.
    /// </summary>
    /// <param name="manifest">Resolved build manifest that identifies the startup scene id.</param>
    /// <returns>Resolved startup-scene entry.</returns>
    static PlatformBuildScene FindStartupScene(PlatformBuildManifest manifest) {
        if (manifest == null) {
            throw new ArgumentNullException(nameof(manifest));
        }

        for (int index = 0; index < manifest.Scenes.Length; index++) {
            PlatformBuildScene scene = manifest.Scenes[index];
            if (string.Equals(scene.SceneId, manifest.StartupSceneId, StringComparison.Ordinal)) {
                return scene;
            }
        }

        throw new InvalidOperationException(
            $"Nintendo DS startup scene '{manifest.StartupSceneId}' was not found in the build manifest.");
    }

    /// <summary>
    /// Reads the cooked runtime-relative payload path stored on one resolved startup-scene entry.
    /// </summary>
    /// <param name="startupScene">Resolved startup-scene entry from the build manifest.</param>
    /// <returns>Cooked runtime-relative payload path that should exist inside NitroFS.</returns>
    static string ReadStartupSceneRelativePath(PlatformBuildScene startupScene) {
        if (startupScene == null) {
            throw new ArgumentNullException(nameof(startupScene));
        }

        for (int index = 0; index < startupScene.ResolvedMetadata.Length; index++) {
            KeyValuePair<string, string> metadataEntry = startupScene.ResolvedMetadata[index];
            if (!string.Equals(metadataEntry.Key, PlatformBuildSceneMetadataKeys.CookedRelativePath, StringComparison.Ordinal)) {
                continue;
            }

            if (string.IsNullOrWhiteSpace(metadataEntry.Value)) {
                break;
            }

            return metadataEntry.Value.Replace('\\', '/');
        }

        throw new InvalidOperationException(
            $"Nintendo DS startup scene '{startupScene.SceneId}' did not define the cooked-relative payload path metadata.");
    }

    /// <summary>
    /// Builds succeeded scene outcomes for the supplied manifest scenes.
    /// </summary>
    /// <param name="scenes">Resolved scenes from the build manifest.</param>
    /// <returns>Per-scene success outcomes.</returns>
    static PlatformBuildItemOutcome[] BuildSceneOutcomes(PlatformBuildScene[] scenes) {
        if (scenes == null || scenes.Length == 0) {
            return [];
        }

        PlatformBuildItemOutcome[] outcomes = new PlatformBuildItemOutcome[scenes.Length];
        for (int index = 0; index < scenes.Length; index++) {
            outcomes[index] = new PlatformBuildItemOutcome(scenes[index].SceneId, PlatformBuildItemOutcomeKind.Succeeded);
        }

        return outcomes;
    }

    /// <summary>
    /// Builds succeeded loose-asset outcomes for the supplied manifest assets.
    /// </summary>
    /// <param name="looseAssets">Resolved loose assets from the build manifest.</param>
    /// <returns>Per-asset success outcomes.</returns>
    static PlatformBuildItemOutcome[] BuildLooseAssetOutcomes(PlatformBuildAsset[] looseAssets) {
        if (looseAssets == null || looseAssets.Length == 0) {
            return [];
        }

        PlatformBuildItemOutcome[] outcomes = new PlatformBuildItemOutcome[looseAssets.Length];
        for (int index = 0; index < looseAssets.Length; index++) {
            outcomes[index] = new PlatformBuildItemOutcome(looseAssets[index].AssetId, PlatformBuildItemOutcomeKind.Succeeded);
        }

        return outcomes;
    }

    /// <summary>
    /// Deletes one directory tree when it already exists so the next staging pass starts from a clean builder-owned workspace.
    /// </summary>
    /// <param name="path">Directory path to clear.</param>
    static void ResetDirectory(string path) {
        if (string.IsNullOrWhiteSpace(path)) {
            throw new ArgumentException("Path must be provided.", nameof(path));
        }

        if (Directory.Exists(path)) {
            Directory.Delete(path, recursive: true);
        }
    }

    /// <summary>
    /// Resolves the Nintendo DS repository root from an environment override or the current working directory.
    /// </summary>
    /// <returns>Absolute Nintendo DS repository root path.</returns>
    static string ResolveRepositoryRootPath() {
        string configuredRepositoryRootPath = Environment.GetEnvironmentVariable(RepositoryRootEnvironmentVariableName) ?? string.Empty;
        if (IsRepositoryRootPath(configuredRepositoryRootPath)) {
            return Path.GetFullPath(configuredRepositoryRootPath);
        }

        string currentPath = Path.GetFullPath(Directory.GetCurrentDirectory());
        while (!string.IsNullOrWhiteSpace(currentPath)) {
            if (IsRepositoryRootPath(currentPath)) {
                return currentPath;
            }

            DirectoryInfo parentDirectory = Directory.GetParent(currentPath);
            if (parentDirectory == null) {
                break;
            }

            currentPath = parentDirectory.FullName;
        }

        throw new InvalidOperationException("Unable to resolve the helengine-ds repository root.");
    }

    /// <summary>
    /// Determines whether the supplied path is the Nintendo DS repository root.
    /// </summary>
    /// <param name="path">Candidate repository root path.</param>
    /// <returns>True when the path matches the expected Nintendo DS repository layout.</returns>
    static bool IsRepositoryRootPath(string path) {
        if (string.IsNullOrWhiteSpace(path)) {
            return false;
        }

        string makefilePath = Path.Combine(path, "Makefile");
        string bootHostPath = Path.Combine(path, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        return File.Exists(makefilePath) && File.Exists(bootHostPath);
    }
}
