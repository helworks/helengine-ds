namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS build freshness guard rejects stale package artifacts.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
public class NintendoDsBuildArtifactFreshnessValidatorTests {
    /// <summary>
    /// Verifies the guard rejects missing artifacts.
    /// </summary>
    [Fact]
    public void EnsureFreshArtifactProduced_whenArtifactIsMissing_throws() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-freshness-" + Guid.NewGuid().ToString("N"));
        try {
            string artifactPath = Path.Combine(workingRoot, "missing.nds");

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                NintendoDsBuildArtifactFreshnessValidator.EnsureFreshArtifactProduced(
                    artifactPath,
                    DateTime.UtcNow,
                    "Nintendo DS package output"));

            Assert.Contains("was not produced", exception.Message);
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the guard rejects artifacts older than the current build invocation.
    /// </summary>
    [Fact]
    public void EnsureFreshArtifactProduced_whenArtifactPredatesBuild_throws() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-freshness-" + Guid.NewGuid().ToString("N"));
        try {
            Directory.CreateDirectory(workingRoot);
            string artifactPath = Path.Combine(workingRoot, "stale.nds");
            File.WriteAllText(artifactPath, "nds");
            DateTime staleWriteTimeUtc = new DateTime(2026, 05, 12, 19, 37, 04, DateTimeKind.Utc);
            File.SetLastWriteTimeUtc(artifactPath, staleWriteTimeUtc);

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                NintendoDsBuildArtifactFreshnessValidator.EnsureFreshArtifactProduced(
                    artifactPath,
                    staleWriteTimeUtc.AddMinutes(1),
                    "Nintendo DS package output"));

            Assert.Contains("is stale", exception.Message);
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the guard accepts artifacts written during the current build invocation.
    /// </summary>
    [Fact]
    public void EnsureFreshArtifactProduced_whenArtifactIsFresh_succeeds() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-freshness-" + Guid.NewGuid().ToString("N"));
        try {
            Directory.CreateDirectory(workingRoot);
            string artifactPath = Path.Combine(workingRoot, "fresh.nds");
            DateTime buildStartedUtc = DateTime.UtcNow;
            File.WriteAllText(artifactPath, "nds");
            File.SetLastWriteTimeUtc(artifactPath, buildStartedUtc.AddSeconds(1));

            NintendoDsBuildArtifactFreshnessValidator.EnsureFreshArtifactProduced(
                artifactPath,
                buildStartedUtc,
                "Nintendo DS package output");
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }
}
