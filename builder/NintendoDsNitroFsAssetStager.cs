using helengine.baseplatform.Manifest;

namespace helengine.ds.builder;

/// <summary>
/// Copies cooked runtime payloads from the package root into the staged Nintendo DS NitroFS tree.
/// </summary>
public sealed class NintendoDsNitroFsAssetStager {
    /// <summary>
    /// Stable scene id used by the generated boot scene that native startup must be able to open through one canonical path.
    /// </summary>
    const string GeneratedBootSceneId = "GeneratedBootScene";

    /// <summary>
    /// Stable canonical Nintendo DS startup-scene path used by the native startup manifest and runtime scene catalog.
    /// </summary>
    const string CanonicalGeneratedBootSceneRelativePath = "cooked/scenes/generatedbootscene.hasset";

    /// <summary>
    /// Identifies the standard cooked-audio directory that DS packaging omits until native audio cooking is supported.
    /// </summary>
    const string CookedAudioRelativePathPrefix = "cooked/audio/";

    /// <summary>
    /// Stages the cooked payload files referenced by the build manifest into NitroFS.
    /// </summary>
    /// <param name="manifest">Build manifest that identifies the cooked payload files to stage.</param>
    /// <param name="sourceRootPath">Package root that already contains the cooked payload files.</param>
    /// <param name="nitroFsRootPath">NitroFS root that should receive the cooked payload files.</param>
    public void Stage(PlatformBuildManifest manifest, string sourceRootPath, string nitroFsRootPath) {
        if (manifest == null) {
            throw new ArgumentNullException(nameof(manifest));
        } else if (string.IsNullOrWhiteSpace(sourceRootPath)) {
            throw new ArgumentException("NitroFS source root path must be provided.", nameof(sourceRootPath));
        } else if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS destination root path must be provided.", nameof(nitroFsRootPath));
        }

        string fullSourceRootPath = Path.GetFullPath(sourceRootPath);
        string fullNitroFsRootPath = Path.GetFullPath(nitroFsRootPath);
        string sourceRootPrefix = EnsureTrailingDirectorySeparator(fullSourceRootPath);
        HashSet<string> stagedRelativePaths = new(StringComparer.OrdinalIgnoreCase);

        Directory.CreateDirectory(fullNitroFsRootPath);

        for (int index = 0; index < manifest.Scenes.Length; index++) {
            PlatformBuildPayloadReference[] payloadReferences = manifest.Scenes[index].PayloadReferences;
            for (int payloadIndex = 0; payloadIndex < payloadReferences.Length; payloadIndex++) {
                StagePayloadReference(payloadReferences[payloadIndex], sourceRootPrefix, fullNitroFsRootPath, stagedRelativePaths);
            }
        }

        for (int index = 0; index < manifest.LooseAssets.Length; index++) {
            StagePayloadReference(manifest.LooseAssets[index].PayloadReference, sourceRootPrefix, fullNitroFsRootPath, stagedRelativePaths);
        }

        PlatformCookWorkItem[] platformCookWorkItems = manifest.PlatformCookWorkItems ?? [];
        for (int index = 0; index < platformCookWorkItems.Length; index++) {
            StageCookWorkItemOutput(platformCookWorkItems[index], sourceRootPrefix, fullNitroFsRootPath, stagedRelativePaths);
        }

        StageCanonicalGeneratedBootSceneAlias(manifest, sourceRootPrefix, fullNitroFsRootPath, stagedRelativePaths);
    }

    /// <summary>
    /// Stages one payload reference into NitroFS when it has not already been copied.
    /// </summary>
    /// <param name="payloadReference">Payload reference that identifies one cooked file.</param>
    /// <param name="sourceRootPrefix">Normalized source-root path with a trailing directory separator.</param>
    /// <param name="nitroFsRootPath">NitroFS root that receives the copied file.</param>
    /// <param name="stagedRelativePaths">Set of already staged relative paths.</param>
    static void StagePayloadReference(
        PlatformBuildPayloadReference payloadReference,
        string sourceRootPrefix,
        string nitroFsRootPath,
        HashSet<string> stagedRelativePaths) {
        if (payloadReference == null) {
            throw new ArgumentNullException(nameof(payloadReference));
        } else if (stagedRelativePaths == null) {
            throw new ArgumentNullException(nameof(stagedRelativePaths));
        }

        string relativePath = NormalizeRelativePath(payloadReference.SourceIdentity);
        if (IsCookedAudioRelativePath(relativePath)) {
            return;
        }

        if (!stagedRelativePaths.Add(relativePath)) {
            return;
        }

        string sourceRootPath = sourceRootPrefix.TrimEnd(Path.DirectorySeparatorChar);
        string sourceFilePath = Path.GetFullPath(Path.Combine(sourceRootPath, relativePath.Replace('/', Path.DirectorySeparatorChar)));
        if (!sourceFilePath.StartsWith(sourceRootPrefix, StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException("Nintendo DS payload paths must stay inside the package root.");
        } else if (!File.Exists(sourceFilePath)) {
            throw new InvalidOperationException($"Nintendo DS payload '{relativePath}' was not found in the package root.");
        }

        string destinationFilePath = Path.Combine(nitroFsRootPath, relativePath.Replace('/', Path.DirectorySeparatorChar));
        string destinationDirectoryPath = Path.GetDirectoryName(destinationFilePath)
            ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS NitroFS destination directory.");
        Directory.CreateDirectory(destinationDirectoryPath);
        File.Copy(sourceFilePath, destinationFilePath, overwrite: true);
    }

    /// <summary>
    /// Stages one builder-owned platform cook work item output into NitroFS when it has not already been copied.
    /// </summary>
    /// <param name="workItem">Builder-owned platform cook work item whose output should be staged.</param>
    /// <param name="sourceRootPrefix">Normalized source-root path with a trailing directory separator.</param>
    /// <param name="nitroFsRootPath">NitroFS root that receives the copied file.</param>
    /// <param name="stagedRelativePaths">Set of already staged relative paths.</param>
    static void StageCookWorkItemOutput(
        PlatformCookWorkItem workItem,
        string sourceRootPrefix,
        string nitroFsRootPath,
        HashSet<string> stagedRelativePaths) {
        if (workItem == null) {
            throw new ArgumentNullException(nameof(workItem));
        }

        if (IsCookedAudioRelativePath(workItem.OutputRelativePath)) {
            return;
        }

        StageRelativePath(workItem.OutputRelativePath, sourceRootPrefix, nitroFsRootPath, stagedRelativePaths);
    }

    /// <summary>
    /// Determines whether one cooked runtime-relative path identifies an audio payload excluded from Nintendo DS packaging.
    /// </summary>
    /// <param name="relativePath">Runtime-relative path to inspect.</param>
    /// <returns><c>true</c> when the path belongs to the cooked audio directory; otherwise <c>false</c>.</returns>
    static bool IsCookedAudioRelativePath(string relativePath) {
        return NormalizeRelativePath(relativePath).StartsWith(CookedAudioRelativePathPrefix, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Stages one relative output path into NitroFS when it has not already been copied.
    /// </summary>
    /// <param name="relativePath">Runtime-relative path that should be copied from the package source root.</param>
    /// <param name="sourceRootPrefix">Normalized source-root path with a trailing directory separator.</param>
    /// <param name="nitroFsRootPath">NitroFS root that receives the copied file.</param>
    /// <param name="stagedRelativePaths">Set of already staged relative paths.</param>
    static void StageRelativePath(
        string relativePath,
        string sourceRootPrefix,
        string nitroFsRootPath,
        HashSet<string> stagedRelativePaths) {
        if (stagedRelativePaths == null) {
            throw new ArgumentNullException(nameof(stagedRelativePaths));
        }

        string normalizedRelativePath = NormalizeRelativePath(relativePath);
        if (!stagedRelativePaths.Add(normalizedRelativePath)) {
            return;
        }

        string sourceRootPath = sourceRootPrefix.TrimEnd(Path.DirectorySeparatorChar);
        string sourceFilePath = Path.GetFullPath(Path.Combine(sourceRootPath, normalizedRelativePath.Replace('/', Path.DirectorySeparatorChar)));
        if (!sourceFilePath.StartsWith(sourceRootPrefix, StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException("Nintendo DS payload paths must stay inside the package root.");
        } else if (!File.Exists(sourceFilePath)) {
            throw new InvalidOperationException($"Nintendo DS payload '{normalizedRelativePath}' was not found in the package root.");
        }

        string destinationFilePath = Path.Combine(nitroFsRootPath, normalizedRelativePath.Replace('/', Path.DirectorySeparatorChar));
        string destinationDirectoryPath = Path.GetDirectoryName(destinationFilePath)
            ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS NitroFS destination directory.");
        Directory.CreateDirectory(destinationDirectoryPath);
        File.Copy(sourceFilePath, destinationFilePath, overwrite: true);
    }

    /// <summary>
    /// Stages one canonical startup-scene alias for the generated boot scene so the native DS startup manifest can open the scene through a stable path.
    /// </summary>
    /// <param name="manifest">Build manifest that may contain the generated boot scene.</param>
    /// <param name="sourceRootPrefix">Normalized source-root path with a trailing directory separator.</param>
    /// <param name="nitroFsRootPath">NitroFS root that receives the copied file.</param>
    /// <param name="stagedRelativePaths">Set of already staged relative paths.</param>
    static void StageCanonicalGeneratedBootSceneAlias(
        PlatformBuildManifest manifest,
        string sourceRootPrefix,
        string nitroFsRootPath,
        HashSet<string> stagedRelativePaths) {
        if (manifest == null) {
            throw new ArgumentNullException(nameof(manifest));
        } else if (stagedRelativePaths == null) {
            throw new ArgumentNullException(nameof(stagedRelativePaths));
        }

        PlatformBuildScene generatedBootScene = Array.Find(
            manifest.Scenes,
            scene => scene != null && string.Equals(scene.SceneId, GeneratedBootSceneId, StringComparison.Ordinal));
        if (generatedBootScene == null) {
            return;
        }

        string cookedRelativePath = ResolveCookedRelativePath(generatedBootScene);
        if (string.Equals(cookedRelativePath, CanonicalGeneratedBootSceneRelativePath, StringComparison.OrdinalIgnoreCase)) {
            return;
        }

        string sourceRootPath = sourceRootPrefix.TrimEnd(Path.DirectorySeparatorChar);
        string sourceFilePath = Path.GetFullPath(Path.Combine(sourceRootPath, cookedRelativePath.Replace('/', Path.DirectorySeparatorChar)));
        if (!sourceFilePath.StartsWith(sourceRootPrefix, StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException("Nintendo DS payload paths must stay inside the package root.");
        } else if (!File.Exists(sourceFilePath)) {
            throw new InvalidOperationException($"Nintendo DS payload '{cookedRelativePath}' was not found in the package root.");
        }

        string normalizedCanonicalRelativePath = NormalizeRelativePath(CanonicalGeneratedBootSceneRelativePath);
        if (!stagedRelativePaths.Add(normalizedCanonicalRelativePath)) {
            return;
        }

        string destinationFilePath = Path.Combine(nitroFsRootPath, normalizedCanonicalRelativePath.Replace('/', Path.DirectorySeparatorChar));
        string destinationDirectoryPath = Path.GetDirectoryName(destinationFilePath)
            ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS NitroFS destination directory.");
        Directory.CreateDirectory(destinationDirectoryPath);
        File.Copy(sourceFilePath, destinationFilePath, overwrite: true);
    }

    /// <summary>
    /// Resolves the cooked runtime-relative scene payload path stored on one manifest scene entry.
    /// </summary>
    /// <param name="scene">Manifest scene entry whose metadata should be read.</param>
    /// <returns>Normalized runtime-relative cooked scene path.</returns>
    static string ResolveCookedRelativePath(PlatformBuildScene scene) {
        if (scene == null) {
            throw new ArgumentNullException(nameof(scene));
        }

        KeyValuePair<string, string>[] resolvedMetadata = scene.ResolvedMetadata ?? [];
        for (int index = 0; index < resolvedMetadata.Length; index++) {
            KeyValuePair<string, string> metadata = resolvedMetadata[index];
            if (!string.Equals(metadata.Key, PlatformBuildSceneMetadataKeys.CookedRelativePath, StringComparison.OrdinalIgnoreCase)) {
                continue;
            } else if (!string.IsNullOrWhiteSpace(metadata.Value)) {
                return NormalizeRelativePath(metadata.Value);
            }
        }

        return NormalizeRelativePath(scene.SourceIdentity);
    }

    /// <summary>
    /// Normalizes one payload path to a forward-slash relative path.
    /// </summary>
    /// <param name="path">Payload path to normalize.</param>
    /// <returns>Normalized forward-slash relative path.</returns>
    static string NormalizeRelativePath(string path) {
        if (string.IsNullOrWhiteSpace(path)) {
            throw new InvalidOperationException("Nintendo DS payload paths must be provided.");
        }

        string normalizedPath = path.Replace('\\', '/');
        if (Path.IsPathRooted(normalizedPath)) {
            throw new InvalidOperationException("Nintendo DS payload paths must not be rooted.");
        } else if (normalizedPath.StartsWith("../", StringComparison.Ordinal) || normalizedPath.Contains("/../", StringComparison.Ordinal)) {
            throw new InvalidOperationException("Nintendo DS payload paths must stay inside the package root.");
        }

        return normalizedPath;
    }

    /// <summary>
    /// Ensures a path ends with one directory separator.
    /// </summary>
    /// <param name="path">Path to normalize.</param>
    /// <returns>Path with a trailing directory separator.</returns>
    static string EnsureTrailingDirectorySeparator(string path) {
        if (string.IsNullOrWhiteSpace(path)) {
            throw new ArgumentException("Path must be provided.", nameof(path));
        }

        return path.EndsWith(Path.DirectorySeparatorChar)
            ? path
            : path + Path.DirectorySeparatorChar;
    }
}
