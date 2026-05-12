namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS startup-manifest writer byte contract and validation behavior.
/// </summary>
public class NintendoDsStartupManifestWriterTests {
    /// <summary>
    /// Verifies the writer emits the expected staged NitroFS path and binary payload.
    /// </summary>
    [Fact]
    public void Write_creates_expected_manifest_bytes() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-manifest-" + Guid.NewGuid().ToString("N"));

        try {
            NintendoDsStartupManifestWriter writer = new();
            string nitroFsRootPath = Path.Combine(workingRoot, "nitrofs");

            string manifestPath = writer.Write(
                nitroFsRootPath,
                topScreenColorHex: "#FF0000",
                bottomScreenColorHex: "#0000FF");

            byte[] bytes = File.ReadAllBytes(manifestPath);

            Assert.Equal(Path.Combine(nitroFsRootPath, "runtime", "ds_startup_manifest.bin"), manifestPath);
            Assert.Equal(
                [
                    0x48, 0x44, 0x53, 0x50,
                    0x01, 0x00,
                    0x04, 0x00,
                    0x1F, 0x80,
                    0x00, 0xFC
                ],
                bytes);
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies invalid authored color text fails with an explicit top-screen setting message.
    /// </summary>
    [Fact]
    public void Write_when_color_text_is_invalid_throws() {
        NintendoDsStartupManifestWriter writer = new();
        string nitroFsRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-manifest-invalid-" + Guid.NewGuid().ToString("N"));

        try {
            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                writer.Write(nitroFsRootPath, "#12", "#0000FF"));
            Assert.Contains("startup top screen color", exception.Message, StringComparison.OrdinalIgnoreCase);
        } finally {
            if (Directory.Exists(nitroFsRootPath)) {
                Directory.Delete(nitroFsRootPath, recursive: true);
            }
        }
    }
}
