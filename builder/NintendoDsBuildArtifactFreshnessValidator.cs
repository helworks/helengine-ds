namespace helengine.ds.builder;

/// <summary>
/// Validates that Nintendo DS build artifacts were freshly produced by the current build invocation.
/// </summary>
internal static class NintendoDsBuildArtifactFreshnessValidator {
    /// <summary>
    /// Verifies that one build artifact exists and was written during or after the current build invocation.
    /// </summary>
    /// <param name="artifactPath">Artifact path that should have been produced.</param>
    /// <param name="buildStartedUtc">UTC timestamp captured immediately before the build started.</param>
    /// <param name="artifactDescription">Human-readable artifact description used in thrown messages.</param>
    public static void EnsureFreshArtifactProduced(string artifactPath, DateTime buildStartedUtc, string artifactDescription) {
        if (string.IsNullOrWhiteSpace(artifactPath)) {
            throw new ArgumentException("Artifact path must be provided.", nameof(artifactPath));
        }
        if (string.IsNullOrWhiteSpace(artifactDescription)) {
            throw new ArgumentException("Artifact description must be provided.", nameof(artifactDescription));
        }
        if (!File.Exists(artifactPath)) {
            throw new InvalidOperationException(artifactDescription + " was not produced.");
        }

        DateTime artifactWriteTimeUtc = File.GetLastWriteTimeUtc(artifactPath);
        if (artifactWriteTimeUtc < buildStartedUtc) {
            throw new InvalidOperationException(
                artifactDescription
                + " is stale. Expected a file written on or after "
                + buildStartedUtc.ToString("O")
                + " but found "
                + artifactWriteTimeUtc.ToString("O")
                + ".");
        }
    }
}
