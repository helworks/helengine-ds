using helengine.baseplatform.Manifest;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS NitroFS stager copies cooked payloads and any required startup-scene aliases into the staged runtime tree.
/// </summary>
public sealed class NintendoDsNitroFsAssetStagerTests {
    /// <summary>
    /// Ensures the stager emits the canonical generated-boot-scene alias expected by the native DS startup manifest.
    /// </summary>
    [Fact]
    public void Stage_whenGeneratedBootSceneUsesHashedCookedPath_stagesCanonicalStartupAlias() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-nitrofs-stager-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "package-source");
        string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
        string hashedRelativePath = "cooked/scenes/.generated-build/ds/buildid/generatedbootscene_buildid.hasset";
        string canonicalRelativePath = "cooked/scenes/generatedbootscene.hasset";
        string payloadContents = "generated-boot-scene";

        try {
            string hashedSourcePath = Path.Combine(sourceRootPath, hashedRelativePath.Replace('/', Path.DirectorySeparatorChar));
            string hashedSourceDirectoryPath = Path.GetDirectoryName(hashedSourcePath)
                ?? throw new InvalidOperationException("Unable to resolve the hashed generated boot scene directory.");
            Directory.CreateDirectory(hashedSourceDirectoryPath);
            File.WriteAllText(hashedSourcePath, payloadContents);

            PlatformBuildScene generatedBootScene = new(
                "GeneratedBootScene",
                "Generated Boot Scene",
                hashedRelativePath,
                [
                    new PlatformBuildPayloadReference(hashedRelativePath, hashedRelativePath)
                ],
                [
                    new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, hashedRelativePath)
                ]);

            PlatformBuildManifest manifest = new(
                1,
                "project",
                "1.0.0",
                "1.0.0",
                "ds",
                "1.0.1",
                "GeneratedBootScene",
                [generatedBootScene],
                Array.Empty<PlatformBuildAsset>(),
                Array.Empty<PlatformBuildArtifact>(),
                Array.Empty<PlatformBuildCodeModule>(),
                Array.Empty<PlatformArtifactPlacement>(),
                new PlatformContainerWritePlan(string.Empty, Array.Empty<PlatformContainerArtifact>()));

            NintendoDsNitroFsAssetStager stager = new();
            stager.Stage(manifest, sourceRootPath, nitroFsRootPath);

            string hashedNitroFsPath = Path.Combine(nitroFsRootPath, hashedRelativePath.Replace('/', Path.DirectorySeparatorChar));
            string canonicalNitroFsPath = Path.Combine(nitroFsRootPath, canonicalRelativePath.Replace('/', Path.DirectorySeparatorChar));
            Assert.True(File.Exists(hashedNitroFsPath));
            Assert.True(File.Exists(canonicalNitroFsPath));
            Assert.Equal(payloadContents, File.ReadAllText(hashedNitroFsPath));
            Assert.Equal(payloadContents, File.ReadAllText(canonicalNitroFsPath));
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, true);
            }
        }
    }
}
