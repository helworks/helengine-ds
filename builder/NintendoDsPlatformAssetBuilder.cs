using helengine;
using helengine.baseplatform.Builders;
using helengine.baseplatform.Descriptors;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Results;
using helengine.baseplatform.Targets;
using helengine.files;
using System.Runtime.Versioning;

namespace helengine.ds.builder;

/// <summary>
/// Implements the Nintendo DS platform asset builder contract.
/// </summary>
[SupportedOSPlatform("windows")]
public sealed class NintendoDsPlatformAssetBuilder : IPlatformAssetBuilder {
    /// <summary>
    /// Environment variable that can override the Nintendo DS repository root.
    /// </summary>
    const string RepositoryRootEnvironmentVariableName = "HELENGINE_DS_REPOSITORY_ROOT";

    /// <summary>
    /// Build-option identifier that controls whether native DS runtime diagnostics remain enabled in the packaged runtime.
    /// </summary>
    const string EnableNativeRuntimeDiagnosticsBuildOptionId = "enable-native-runtime-diagnostics";

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
    /// Imports source assets and converts them into final runtime payloads for builder-owned Nintendo DS cook work items.
    /// </summary>
    readonly INintendoDsPlatformCookSourceProcessor PlatformCookSourceProcessor;

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
        PlatformCookSourceProcessor = new NintendoDsPlatformCookSourceProcessor();
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
        PlatformCookSourceProcessor = new NintendoDsPlatformCookSourceProcessor();
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    /// <summary>
    /// Initializes one Nintendo DS builder with explicit native-build and source-cook seams for tests.
    /// </summary>
    /// <param name="nativeBuildExecutor">Native build executor override used by tests.</param>
    /// <param name="repositoryRootPath">Repository root override used by tests.</param>
    /// <param name="platformCookSourceProcessor">Source asset cook processor used for builder-owned Nintendo DS work items.</param>
    internal NintendoDsPlatformAssetBuilder(
        INintendoDsNativeBuildExecutor nativeBuildExecutor,
        string repositoryRootPath,
        INintendoDsPlatformCookSourceProcessor platformCookSourceProcessor) {
        NativeBuildExecutor = nativeBuildExecutor ?? throw new ArgumentNullException(nameof(nativeBuildExecutor));
        RepositoryRootPath = string.IsNullOrWhiteSpace(repositoryRootPath)
            ? throw new ArgumentException("Repository root path must be provided.", nameof(repositoryRootPath))
            : Path.GetFullPath(repositoryRootPath);
        StartupManifestWriter = new NintendoDsStartupManifestWriter();
        GeneratedCoreStager = new NintendoDsGeneratedCoreStager();
        NitroFsAssetStager = new NintendoDsNitroFsAssetStager();
        SceneAssetSanitizer = new NintendoDsSceneAssetSanitizer();
        PlatformCookSourceProcessor = platformCookSourceProcessor ?? throw new ArgumentNullException(nameof(platformCookSourceProcessor));
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
    /// Cooks one Nintendo DS fixed-pipeline material payload for the generic cooked-platform-owned runtime seam.
    /// </summary>
    /// <param name="request">Material cook request to translate.</param>
    /// <returns>Cooked Nintendo DS material payload.</returns>
    public PlatformMaterialCookResult CookMaterial(PlatformMaterialCookRequest request) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        } else if (!string.Equals(request.TargetPlatformId, "ds", StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException($"Nintendo DS cannot cook materials for target platform '{request.TargetPlatformId}'.");
        } else if (!string.Equals(request.SchemaId, NintendoDsMaterialSchemaIds.StandardTexturedSchemaId, StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException($"Nintendo DS does not support material schema '{request.SchemaId}'.");
        }

        ResolveBaseColor(
            request.FieldValues,
            out byte baseColorRed,
            out byte baseColorGreen,
            out byte baseColorBlue,
            out byte baseColorAlpha);
        PlatformMaterialAsset cookedAsset = new PlatformMaterialAsset {
            Id = request.MaterialAssetId,
            RendererFamilyId = string.IsNullOrWhiteSpace(request.SelectedGraphicsProfileId)
                ? throw new InvalidOperationException("Nintendo DS material cooking requires a graphics profile id.")
                : request.SelectedGraphicsProfileId,
            TextureRelativePath = ResolveOptionalString(request.FieldValues, NintendoDsMaterialSchemaIds.TextureRelativePathFieldId),
            DoubleSided = ResolveBoolean(request.FieldValues, NintendoDsMaterialSchemaIds.DoubleSidedFieldId),
            UseVertexColor = ResolveVertexColorMode(request.FieldValues),
            Lit = ResolveLightingMode(request.FieldValues),
            BaseColorR = baseColorRed,
            BaseColorG = baseColorGreen,
            BaseColorB = baseColorBlue,
            BaseColorA = baseColorAlpha
        };

        return new PlatformMaterialCookResult(
            helengine.files.AssetSerializer.SerializeToBytes(cookedAsset),
            []);
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
        bool enableRuntimeDiagnostics = ReadOptionalBooleanBuildOption(
            request.SelectedBuildOptionValues,
            EnableNativeRuntimeDiagnosticsBuildOptionId,
            false);
        string disabledRuntimeFeatures = ReadOptionalStringBuildOption(
            request.SelectedCodegenOptionValues,
            PlatformCodegenSettingIds.ForcedDisabledFeatures,
            string.Empty);
        string repositoryRootPath = string.IsNullOrWhiteSpace(RepositoryRootPath)
            ? ResolveRepositoryRootPath()
            : RepositoryRootPath;

        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);

        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRootPath,
            request.WorkingRoot,
            request.OutputRoot,
            request.GeneratedCoreCppRootPath,
            enableRuntimeDiagnostics,
            disabledRuntimeFeatures);
        string packageSourceRootPath = NintendoDsBuildPathConventions.ResolvePackageSourceRootPath(request.WorkingRoot);
        ValidatePackageSourceRootPath(packageSourceRootPath);
        ExecutePlatformCookWorkItems(request, packageSourceRootPath, progressReporter, cancellationToken);

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
    /// Executes builder-owned Nintendo DS cook work items directly into the staged package source root using the editor-resolved source path and serialized settings payload.
    /// </summary>
    /// <param name="request">Resolved build request.</param>
    /// <param name="packageSourceRootPath">Builder-owned staged package source root that receives builder-owned cooked outputs.</param>
    /// <param name="progressReporter">Streaming progress reporter.</param>
    /// <param name="cancellationToken">Cancellation token.</param>
    void ExecutePlatformCookWorkItems(
        PlatformBuildRequest request,
        string packageSourceRootPath,
        IPlatformBuildProgressReporter progressReporter,
        CancellationToken cancellationToken) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        } else if (string.IsNullOrWhiteSpace(packageSourceRootPath)) {
            throw new ArgumentException("Package source root path must be provided.", nameof(packageSourceRootPath));
        } else if (progressReporter == null) {
            throw new ArgumentNullException(nameof(progressReporter));
        }

        PlatformCookWorkItem[] platformCookWorkItems = request.Manifest.PlatformCookWorkItems ?? [];
        for (int workItemIndex = 0; workItemIndex < platformCookWorkItems.Length; workItemIndex++) {
            cancellationToken.ThrowIfCancellationRequested();

            PlatformCookWorkItem workItem = platformCookWorkItems[workItemIndex];
            ExecutePlatformCookWorkItem(workItem, packageSourceRootPath);
            progressReporter.Report(new PlatformBuildProgressUpdate(
                "Execute Platform Cook Work Items",
                workItem.OutputLogicalArtifactId,
                workItemIndex + 1,
                platformCookWorkItems.Length,
                $"Cooked platform-owned artifact '{workItem.OutputRelativePath}'."));
        }
    }

    /// <summary>
    /// Executes one builder-owned Nintendo DS cook work item and writes the final runtime payload into the exact declared output path.
    /// </summary>
    /// <param name="workItem">Builder-owned Nintendo DS cook work item to execute.</param>
    /// <param name="packageSourceRootPath">Builder-owned staged package source root that receives the cooked output.</param>
    void ExecutePlatformCookWorkItem(PlatformCookWorkItem workItem, string packageSourceRootPath) {
        if (workItem == null) {
            throw new ArgumentNullException(nameof(workItem));
        } else if (string.IsNullOrWhiteSpace(packageSourceRootPath)) {
            throw new ArgumentException("Package source root path must be provided.", nameof(packageSourceRootPath));
        } else if (!string.Equals(workItem.TargetPlatformId, Definition.PlatformId, StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException($"Unsupported Nintendo DS work item target platform '{workItem.TargetPlatformId}'.");
        }

        TextureAssetProcessorSettings settings = NintendoDsTextureCookSettingsSerializer.Deserialize(workItem.SerializedPlatformSettings);
        string assetId = ResolveCookWorkItemAssetId(workItem);
        string destinationPath = Path.Combine(packageSourceRootPath, NormalizeRelativePath(workItem.OutputRelativePath));
        string destinationDirectoryPath = Path.GetDirectoryName(destinationPath)
            ?? throw new InvalidOperationException($"Destination directory could not be resolved for '{destinationPath}'.");
        Directory.CreateDirectory(destinationDirectoryPath);

        if (string.Equals(workItem.SourceAssetKind, "texture", StringComparison.OrdinalIgnoreCase)) {
            TextureAsset cookedTextureAsset = PlatformCookSourceProcessor.CookTexture(workItem.SourceAssetPath, assetId, settings);
            File.WriteAllBytes(destinationPath, global::helengine.files.AssetSerializer.SerializeToBytes(cookedTextureAsset));
            return;
        } else if (string.Equals(workItem.SourceAssetKind, "font-atlas-texture", StringComparison.OrdinalIgnoreCase)) {
            string fontAtlasSourcePath = ResolveFontAtlasCookSourcePath(workItem, packageSourceRootPath);
            TextureAsset cookedFontAtlasTextureAsset = PlatformCookSourceProcessor.CookFontAtlasTexture(fontAtlasSourcePath, assetId, settings);
            File.WriteAllBytes(destinationPath, global::helengine.files.AssetSerializer.SerializeToBytes(cookedFontAtlasTextureAsset));
            return;
        }

        throw new InvalidOperationException($"Unsupported Nintendo DS platform cook work item source kind '{workItem.SourceAssetKind}'.");
    }

    /// <summary>
    /// Resolves the preferred source path for one font-atlas cook work item.
    /// </summary>
    /// <param name="workItem">Builder-owned Nintendo DS cook work item to execute.</param>
    /// <param name="packageSourceRootPath">Builder-owned staged package source root that may already contain the packaged font asset.</param>
    /// <returns>Preferred source path for atlas cooking.</returns>
    static string ResolveFontAtlasCookSourcePath(PlatformCookWorkItem workItem, string packageSourceRootPath) {
        if (workItem == null) {
            throw new ArgumentNullException(nameof(workItem));
        } else if (string.IsNullOrWhiteSpace(packageSourceRootPath)) {
            throw new ArgumentException("Package source root path must be provided.", nameof(packageSourceRootPath));
        } else if (string.IsNullOrWhiteSpace(workItem.SourceAssetPath)) {
            throw new InvalidOperationException("Nintendo DS font-atlas work items must provide a source asset path.");
        } else if (string.IsNullOrWhiteSpace(workItem.OutputRelativePath)) {
            throw new InvalidOperationException("Nintendo DS font-atlas work items must provide an output relative path.");
        }

        string packagedFontRelativePath = Path.ChangeExtension(NormalizeRelativePath(workItem.OutputRelativePath), ".hefont");
        string packagedFontPath = Path.Combine(packageSourceRootPath, packagedFontRelativePath);
        if (PackagedFontAssetProvidesEmbeddedAtlasTexture(packagedFontPath)) {
            return packagedFontPath;
        }

        return workItem.SourceAssetPath;
    }

    /// <summary>
    /// Determines whether one staged packaged font still contains embedded source-atlas texture bytes.
    /// </summary>
    /// <param name="packagedFontPath">Absolute staged packaged font path to inspect.</param>
    /// <returns>True when the staged packaged font exposes one embedded source atlas texture payload; otherwise false.</returns>
    static bool PackagedFontAssetProvidesEmbeddedAtlasTexture(string packagedFontPath) {
        if (string.IsNullOrWhiteSpace(packagedFontPath)) {
            throw new ArgumentException("Packaged font path must be provided.", nameof(packagedFontPath));
        }
        if (!File.Exists(packagedFontPath)) {
            return false;
        }

        using FileStream stream = new(packagedFontPath, FileMode.Open, FileAccess.Read, FileShare.Read);
        FontAsset fontAsset = global::helengine.files.FontAssetBinarySerializer.Deserialize(stream);
        return fontAsset.SourceTextureAsset != null;
    }

    /// <summary>
    /// Creates the standard builder descriptor shared by both constructors.
    /// </summary>
    /// <returns>The Nintendo DS builder descriptor.</returns>
    static PlatformBuilderDescriptor CreateDescriptor() {
        return new PlatformBuilderDescriptor(
            "helengine.ds.builder",
            "1.0.1",
            "ds",
            new EngineCompatibilityRange("1.0.0", "999.0.0"),
            new ManifestCompatibilityRange(1, 3),
            ["ds"],
            ["release", "debug"]);
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
        }

        PlatformBuildScene startupScene = FindNintendoDsStartupScene(manifest);
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
    /// Reads one optional Nintendo DS boolean build option value.
    /// </summary>
    /// <param name="values">Selected build option values keyed by option id.</param>
    /// <param name="key">Build option id to resolve.</param>
    /// <param name="defaultValue">Value to use when the option is omitted.</param>
    /// <returns>Resolved build option value.</returns>
    static bool ReadOptionalBooleanBuildOption(IReadOnlyDictionary<string, string> values, string key, bool defaultValue) {
        if (values == null) {
            throw new ArgumentNullException(nameof(values));
        } else if (string.IsNullOrWhiteSpace(key)) {
            throw new ArgumentException("Build option id must be provided.", nameof(key));
        }

        if (!values.TryGetValue(key, out string value) || string.IsNullOrWhiteSpace(value)) {
            return defaultValue;
        } else if (bool.TryParse(value, out bool parsedValue)) {
            return parsedValue;
        }

        throw new InvalidOperationException($"Nintendo DS build option '{key}' must be either 'true' or 'false'.");
    }

    /// <summary>
    /// Reads one optional string build or codegen option and falls back to the supplied default value when the option is omitted.
    /// </summary>
    /// <param name="values">Selected option values keyed by option id.</param>
    /// <param name="key">Option id to resolve.</param>
    /// <param name="defaultValue">Value to use when the option is omitted.</param>
    /// <returns>Resolved option value.</returns>
    static string ReadOptionalStringBuildOption(IReadOnlyDictionary<string, string> values, string key, string defaultValue) {
        if (values == null) {
            throw new ArgumentNullException(nameof(values));
        } else if (string.IsNullOrWhiteSpace(key)) {
            throw new ArgumentException("Build option id must be provided.", nameof(key));
        }

        return values.TryGetValue(key, out string value) && value != null
            ? value
            : defaultValue;
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
    /// Finds the Nintendo DS-generated boot scene that must be staged into NitroFS.
    /// </summary>
    /// <param name="manifest">Resolved build manifest that should contain the generated boot scene.</param>
    /// <returns>Resolved Nintendo DS startup-scene entry.</returns>
    static PlatformBuildScene FindNintendoDsStartupScene(PlatformBuildManifest manifest) {
        if (manifest == null) {
            throw new ArgumentNullException(nameof(manifest));
        } else if (string.IsNullOrWhiteSpace(manifest.StartupSceneId)) {
            throw new InvalidOperationException("Nintendo DS build manifests must declare a startup scene id.");
        }

        for (int index = 0; index < manifest.Scenes.Length; index++) {
            PlatformBuildScene scene = manifest.Scenes[index];
            if (string.Equals(scene.SceneId, manifest.StartupSceneId, StringComparison.Ordinal)) {
                return scene;
            }
        }

        throw new InvalidOperationException(
            $"Nintendo DS requires startup scene '{manifest.StartupSceneId}' to be present in the build manifest.");
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
    /// Resolves the stable runtime asset id the builder should assign to one builder-owned cooked output.
    /// </summary>
    /// <param name="workItem">Builder-owned Nintendo DS cook work item whose metadata should be interpreted.</param>
    /// <returns>Stable runtime asset identifier for the cooked payload.</returns>
    static string ResolveCookWorkItemAssetId(PlatformCookWorkItem workItem) {
        if (workItem == null) {
            throw new ArgumentNullException(nameof(workItem));
        }

        PlatformCookWorkItemMetadata[] metadata = workItem.Metadata ?? [];
        for (int index = 0; index < metadata.Length; index++) {
            PlatformCookWorkItemMetadata entry = metadata[index];
            if (entry == null) {
                continue;
            }

            if (string.Equals(entry.Key, "source-asset-id", StringComparison.OrdinalIgnoreCase) && !string.IsNullOrWhiteSpace(entry.Value)) {
                return entry.Value;
            }
        }

        return workItem.OutputRelativePath;
    }

    /// <summary>
    /// Normalizes one relative filesystem path to the current directory separator convention.
    /// </summary>
    /// <param name="relativePath">Relative path to normalize.</param>
    /// <returns>Normalized filesystem path.</returns>
    static string NormalizeRelativePath(string relativePath) {
        if (string.IsNullOrWhiteSpace(relativePath)) {
            throw new ArgumentException("Relative path must be provided.", nameof(relativePath));
        }

        return relativePath.Replace('\\', Path.DirectorySeparatorChar).Replace('/', Path.DirectorySeparatorChar);
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

        string repositoryRootPathFromAssemblyLocation = ResolveRepositoryRootPathFromAssemblyLocation();
        if (!string.IsNullOrWhiteSpace(repositoryRootPathFromAssemblyLocation)) {
            return repositoryRootPathFromAssemblyLocation;
        }

        throw new InvalidOperationException("Unable to resolve the helengine-ds repository root.");
    }

    /// <summary>
    /// Resolves the Nintendo DS repository root by walking upward from the loaded builder assembly location.
    /// </summary>
    /// <returns>Absolute Nintendo DS repository root path when it can be resolved; otherwise an empty string.</returns>
    static string ResolveRepositoryRootPathFromAssemblyLocation() {
        string assemblyLocation = typeof(NintendoDsPlatformAssetBuilder).Assembly.Location;
        if (string.IsNullOrWhiteSpace(assemblyLocation)) {
            return string.Empty;
        }

        string assemblyDirectoryPath = Path.GetDirectoryName(assemblyLocation) ?? string.Empty;
        if (string.IsNullOrWhiteSpace(assemblyDirectoryPath)) {
            return string.Empty;
        }

        string currentPath = Path.GetFullPath(assemblyDirectoryPath);
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

        return string.Empty;
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

    /// <summary>
    /// Reads one required boolean field from the material cook request.
    /// </summary>
    /// <param name="fieldValues">Material field values keyed by field id.</param>
    /// <param name="fieldId">Field identifier to resolve.</param>
    /// <returns>Resolved boolean value.</returns>
    static bool ResolveBoolean(IReadOnlyDictionary<string, string> fieldValues, string fieldId) {
        string value = ResolveRequiredString(fieldValues, fieldId);
        if (string.Equals(value, "true", StringComparison.OrdinalIgnoreCase)) {
            return true;
        } else if (string.Equals(value, "false", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        throw new InvalidOperationException($"Nintendo DS material field '{fieldId}' must be 'true' or 'false'.");
    }

    /// <summary>
    /// Resolves whether vertex colors should modulate the final output color.
    /// </summary>
    /// <param name="fieldValues">Material field values keyed by field id.</param>
    /// <returns>True when vertex color modulation should remain enabled.</returns>
    static bool ResolveVertexColorMode(IReadOnlyDictionary<string, string> fieldValues) {
        string value = ResolveRequiredString(fieldValues, NintendoDsMaterialSchemaIds.VertexColorModeFieldId);
        if (string.Equals(value, "multiply", StringComparison.OrdinalIgnoreCase)) {
            return true;
        } else if (string.Equals(value, "ignore", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        throw new InvalidOperationException("Nintendo DS material field 'vertex-color-mode' must be 'multiply' or 'ignore'.");
    }

    /// <summary>
    /// Resolves whether the cooked material should participate in lighting.
    /// </summary>
    /// <param name="fieldValues">Material field values keyed by field id.</param>
    /// <returns>True when lighting should remain enabled.</returns>
    static bool ResolveLightingMode(IReadOnlyDictionary<string, string> fieldValues) {
        string value = ResolveRequiredString(fieldValues, NintendoDsMaterialSchemaIds.LightingModeFieldId);
        if (string.Equals(value, "lit", StringComparison.OrdinalIgnoreCase)) {
            return true;
        } else if (string.Equals(value, "unlit", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        throw new InvalidOperationException("Nintendo DS material field 'lighting-mode' must be 'lit' or 'unlit'.");
    }

    /// <summary>
    /// Resolves one optional string field from the material cook request.
    /// </summary>
    /// <param name="fieldValues">Material field values keyed by field id.</param>
    /// <param name="fieldId">Field identifier to resolve.</param>
    /// <returns>Resolved string value, or an empty string when the field is absent.</returns>
    static string ResolveOptionalString(IReadOnlyDictionary<string, string> fieldValues, string fieldId) {
        if (fieldValues == null) {
            throw new ArgumentNullException(nameof(fieldValues));
        } else if (string.IsNullOrWhiteSpace(fieldId)) {
            throw new ArgumentException("Field id must be provided.", nameof(fieldId));
        }

        if (!fieldValues.TryGetValue(fieldId, out string value) || string.IsNullOrWhiteSpace(value)) {
            return string.Empty;
        }

        return value;
    }

    /// <summary>
    /// Resolves one required string field from the material cook request.
    /// </summary>
    /// <param name="fieldValues">Material field values keyed by field id.</param>
    /// <param name="fieldId">Field identifier to resolve.</param>
    /// <returns>Resolved non-empty field value.</returns>
    static string ResolveRequiredString(IReadOnlyDictionary<string, string> fieldValues, string fieldId) {
        if (fieldValues == null) {
            throw new ArgumentNullException(nameof(fieldValues));
        } else if (string.IsNullOrWhiteSpace(fieldId)) {
            throw new ArgumentException("Field id must be provided.", nameof(fieldId));
        }

        if (!fieldValues.TryGetValue(fieldId, out string value) || string.IsNullOrWhiteSpace(value)) {
            throw new InvalidOperationException($"Nintendo DS material field '{fieldId}' is required.");
        }

        return value;
    }

    /// <summary>
    /// Resolves one HTML-style base color value into packed byte channels.
    /// </summary>
    /// <param name="fieldValues">Material field values keyed by field id.</param>
    /// <param name="red">Resolved red channel.</param>
    /// <param name="green">Resolved green channel.</param>
    /// <param name="blue">Resolved blue channel.</param>
    /// <param name="alpha">Resolved alpha channel.</param>
    static void ResolveBaseColor(
        IReadOnlyDictionary<string, string> fieldValues,
        out byte red,
        out byte green,
        out byte blue,
        out byte alpha) {
        string value = ResolveRequiredString(fieldValues, NintendoDsMaterialSchemaIds.BaseColorFieldId);
        if (value.Length == 7 && value[0] == '#') {
            red = ParseHexByte(value, 1);
            green = ParseHexByte(value, 3);
            blue = ParseHexByte(value, 5);
            alpha = 255;
            return;
        } else if (value.Length == 9 && value[0] == '#') {
            red = ParseHexByte(value, 1);
            green = ParseHexByte(value, 3);
            blue = ParseHexByte(value, 5);
            alpha = ParseHexByte(value, 7);
            return;
        }

        throw new InvalidOperationException("Nintendo DS material field 'base-color' must use #RRGGBB or #RRGGBBAA.");
    }

    /// <summary>
    /// Parses one hexadecimal byte from a serialized color string.
    /// </summary>
    /// <param name="value">Serialized color string.</param>
    /// <param name="startIndex">Zero-based character index of the first hexadecimal digit.</param>
    /// <returns>Resolved byte value.</returns>
    static byte ParseHexByte(string value, int startIndex) {
        if (string.IsNullOrWhiteSpace(value)) {
            throw new ArgumentException("Color value must be provided.", nameof(value));
        }

        string segment = value.Substring(startIndex, 2);
        if (byte.TryParse(segment, System.Globalization.NumberStyles.HexNumber, System.Globalization.CultureInfo.InvariantCulture, out byte parsedValue)) {
            return parsedValue;
        }

        throw new InvalidOperationException($"Nintendo DS material color value '{value}' contains invalid hexadecimal digits.");
    }
}

