namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS build artifact guard validates deterministic build outputs without comparing clocks across host and Docker.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
public class NintendoDsBuildArtifactFreshnessValidatorTests {
    /// <summary>
    /// Verifies the guard rejects missing artifacts.
    /// </summary>
    [Fact]
    public void EnsureArtifactProduced_whenArtifactIsMissing_throws() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-freshness-" + Guid.NewGuid().ToString("N"));
        try {
            string artifactPath = Path.Combine(workingRoot, "missing.nds");

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
                    artifactPath,
                    "Nintendo DS package output"));

            Assert.Contains("was not produced", exception.Message);
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the guard rejects an empty artifact even when the file exists.
    /// </summary>
    [Fact]
    public void EnsureArtifactProduced_whenArtifactIsEmpty_throws() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-freshness-" + Guid.NewGuid().ToString("N"));
        try {
            Directory.CreateDirectory(workingRoot);
            string artifactPath = Path.Combine(workingRoot, "empty.nds");
            File.WriteAllBytes(artifactPath, Array.Empty<byte>());

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
                    artifactPath,
                    "Nintendo DS package output"));

            Assert.Contains("empty", exception.Message);
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the guard accepts a valid artifact regardless of host and Docker clock differences.
    /// </summary>
    [Fact]
    public void EnsureArtifactProduced_whenArtifactHasAnOlderTimestamp_succeeds() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-freshness-" + Guid.NewGuid().ToString("N"));
        try {
            Directory.CreateDirectory(workingRoot);
            string artifactPath = Path.Combine(workingRoot, "artifact.nds");
            File.WriteAllText(artifactPath, "nds");
            File.SetLastWriteTimeUtc(artifactPath, new DateTime(2026, 05, 12, 19, 37, 04, DateTimeKind.Utc));

            NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
                artifactPath,
                "Nintendo DS package output");
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }
}
