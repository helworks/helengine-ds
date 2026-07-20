namespace helengine.ds.builder;

/// <summary>
/// Validates the deterministic output contract of one completed Nintendo DS native build.
/// </summary>
internal static class NintendoDsBuildArtifactValidator {
    /// <summary>
    /// Verifies that one required build artifact exists and contains data.
    /// </summary>
    /// <param name="artifactPath">Artifact path that should have been produced.</param>
    /// <param name="artifactDescription">Human-readable artifact description used in thrown messages.</param>
    public static void EnsureArtifactProduced(string artifactPath, string artifactDescription) {
        if (string.IsNullOrWhiteSpace(artifactPath)) {
            throw new ArgumentException("Artifact path must be provided.", nameof(artifactPath));
        }
        if (string.IsNullOrWhiteSpace(artifactDescription)) {
            throw new ArgumentException("Artifact description must be provided.", nameof(artifactDescription));
        }
        if (!File.Exists(artifactPath)) {
            throw new InvalidOperationException(artifactDescription + " was not produced.");
        }

        FileInfo artifactInfo = new FileInfo(artifactPath);
        if (artifactInfo.Length <= 0) {
            throw new InvalidOperationException(artifactDescription + " was produced as an empty file.");
        }
    }
}
