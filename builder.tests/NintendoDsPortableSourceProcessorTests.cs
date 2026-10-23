namespace helengine.ds.builder.tests;

/// <summary>Verifies real source image and font cooking on hosts without Windows GDI.</summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
public sealed class NintendoDsPortableSourceProcessorTests {
    /// <summary>Preserves the decoded PNG pixel and stable runtime identity through the real source processor.</summary>
    [Fact]
    public void CookTexture_imports_png_on_the_current_host() {
        string directory = Path.Combine(Path.GetTempPath(), "portable-import");
        Directory.CreateDirectory(directory);
        string source = Path.Combine(directory, "source.png");
        File.WriteAllBytes(source, Convert.FromBase64String("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR4nGP4z8DwHwAFAAH/iZk9HQAAAABJRU5ErkJggg=="));
        try {
            using TextureAsset cooked = new NintendoDsPlatformCookSourceProcessor().CookTexture(source, "texture:portable", Settings());
            Assert.Equal((ushort)1, cooked.Width);
            Assert.Equal((ushort)1, cooked.Height);
            Assert.Equal("texture:portable", cooked.Id);
            Assert.Equal(RuntimeAssetIdGenerator.Generate(cooked.Id), cooked.RuntimeAssetId);
            Assert.Equal(new byte[] { 255, 0, 0, 255 }, cooked.Colors);
        } finally {
            File.Delete(source);
            Directory.Delete(directory);
        }
    }

    /// <summary>Rasterizes the explicit source font and preserves the atlas identifier through real source cooking.</summary>
    [Fact]
    public void CookFontAtlasTexture_imports_source_font_on_the_current_host() {
        string source = Path.Combine(AppContext.BaseDirectory, "fixtures", "Synthetic.ttf");
        using TextureAsset cooked = new NintendoDsPlatformCookSourceProcessor().CookFontAtlasTexture(source, "font:portable", Settings());
        Assert.Equal("font:portable#atlas", cooked.Id);
        Assert.Equal(RuntimeAssetIdGenerator.Generate(cooked.Id), cooked.RuntimeAssetId);
        Assert.Equal(TextureAssetColorFormat.Rgba32, cooked.ColorFormat);
        Assert.True(cooked.Width > 0 && cooked.Height > 0);
        Assert.Contains(cooked.Colors, channel => channel != 0);
    }

    /// <summary>Selects lossless generic RGBA so source import pixels remain directly observable.</summary>
    static TextureAssetProcessorSettings Settings() => new TextureAssetProcessorSettings {
        ColorFormat = TextureAssetColorFormat.Rgba32,
        AlphaPrecision = TextureAssetAlphaPrecision.A8,
        MaxResolution = 0
    };
}
