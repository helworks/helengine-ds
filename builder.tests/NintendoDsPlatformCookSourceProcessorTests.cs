using System.Runtime.Versioning;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies Nintendo DS builder-owned source processing for texture and font-atlas cook work items.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
[SupportedOSPlatform("windows")]
public sealed class NintendoDsPlatformCookSourceProcessorTests {
    /// <summary>
    /// Verifies font-atlas cooking accepts one generated serialized texture asset source written as <c>.hasset</c> and applies the requested Nintendo DS texture settings.
    /// </summary>
    [Fact]
    public void CookFontAtlasTexture_when_source_is_serialized_texture_hasset_applies_requested_texture_settings() {
        string temporaryDirectoryPath = Path.Combine(Path.GetTempPath(), "helengine-ds-font-atlas-cook-tests", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(temporaryDirectoryPath);
        string sourceAssetPath = Path.Combine(temporaryDirectoryPath, "default-font-atlas.hasset");
        TextureAsset sourceTextureAsset = new() {
            Id = "generated:editor:default-font-atlas",
            Width = 4,
            Height = 1,
            Colors = [255, 255, 255, 255, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255],
            PaletteColors = Array.Empty<byte>(),
            ColorFormat = TextureAssetColorFormat.Rgba32,
            AlphaPrecision = TextureAssetAlphaPrecision.A8
        };
        TextureAssetProcessorSettings settings = new TextureAssetProcessorSettings {
            MaxResolution = 0,
            ColorFormat = TextureAssetColorFormat.Indexed4,
            AlphaPrecision = TextureAssetAlphaPrecision.Binary
        };

        try {
            File.WriteAllBytes(sourceAssetPath, global::helengine.files.AssetSerializer.SerializeToBytes(sourceTextureAsset));

            NintendoDsPlatformCookSourceProcessor processor = new();
            TextureAsset cookedTextureAsset = processor.CookFontAtlasTexture(sourceAssetPath, "ui-font", settings);

            Assert.Equal("ui-font#atlas", cookedTextureAsset.Id);
            Assert.Equal(RuntimeAssetIdGenerator.Generate("ui-font#atlas"), cookedTextureAsset.RuntimeAssetId);
            Assert.Equal(sourceTextureAsset.Width, cookedTextureAsset.Width);
            Assert.Equal(sourceTextureAsset.Height, cookedTextureAsset.Height);
            Assert.Equal(TextureAssetColorFormat.Indexed4, cookedTextureAsset.ColorFormat);
            Assert.Equal(TextureAssetAlphaPrecision.Binary, cookedTextureAsset.AlphaPrecision);
            Assert.NotEmpty(cookedTextureAsset.Colors);
            Assert.NotEmpty(cookedTextureAsset.PaletteColors);
        } finally {
            if (Directory.Exists(temporaryDirectoryPath)) {
                Directory.Delete(temporaryDirectoryPath, recursive: true);
            }
        }
    }
}
