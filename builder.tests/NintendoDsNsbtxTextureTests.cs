using helengine.editor;
using System.Runtime.Versioning;

namespace helengine.ds.builder.tests;

/// <summary>Checks NSBTX import against an independent ndspy corpus, including all four compressed interpolation modes.</summary>
public sealed class NintendoDsNsbtxTextureTests {
    /// <summary>Compares every imported channel with independently rendered native DS pixels.</summary>
    [Theory]
    [InlineData("a3i5")]
    [InlineData("indexed2")]
    [InlineData("indexed4")]
    [InlineData("indexed8")]
    [InlineData("compressed4x4")]
    [InlineData("a5i3")]
    [InlineData("bgr5551")]
    public void Independent_archives_match_ndspy_pixels(string name) {
        using FileStream stream = File.OpenRead(Path.Combine(Root(), name + ".nsbtx"));
        using TextureAsset imported = new NintendoDsNsbtxTextureImporter().ImportTexture(stream);
        Assert.Equal(8, imported.Width); Assert.Equal(8, imported.Height); Assert.Equal(name, imported.Id);
        Assert.Equal(File.ReadAllBytes(Path.Combine(Root(), name + ".rgba")), imported.Colors);
        stream.Position = 0;
        using TextureAsset native = new NintendoDsNsbtxTextureImporter().ImportNativeTexture(stream);
        Assert.Equal(imported.Colors, TextureAssetPixelCodec.DecodeToRgba32(native));
    }
    /// <summary>Checks named palette association, explicit selection and invalid selection bounds in a multi-texture archive.</summary>
    [Fact]
    public void Multi_texture_archives_select_matching_palettes() {
        string[] names = new[] { "a3i5", "indexed2", "indexed4", "indexed8", "compressed4x4", "a5i3", "bgr5551" };
        for (int index = 0; index < names.Length; index++) {
            using FileStream stream = File.OpenRead(Path.Combine(Root(), "multi.nsbtx"));
            using TextureAsset native = new NintendoDsNsbtxTextureImporter().ImportNativeTexture(stream, index);
            Assert.Equal(File.ReadAllBytes(Path.Combine(Root(), names[index] + ".rgba")), NintendoDsTextureCodec.Decode(native));
        }
        using FileStream invalid = File.OpenRead(Path.Combine(Root(), "multi.nsbtx"));
        Assert.Throws<InvalidDataException>(() => new NintendoDsNsbtxTextureImporter().ImportNativeTexture(invalid, 7));
        invalid.Position = 0;
        Assert.Throws<InvalidDataException>(() => new NintendoDsNsbtxTextureImporter().ImportNativeTexture(invalid, 0, 255));

        byte[] ambiguous = File.ReadAllBytes(Path.Combine(Root(), "multi.nsbtx"));
        int section = (int)NintendoDsTextureCodec.Read(ambiguous, 16);
        int dictionary = section + (int)NintendoDsTextureCodec.Read(ambiguous, section + 52);
        int entries = dictionary + NintendoDsTextureCodec.Word(ambiguous, dictionary + 6);
        int paletteName = entries + NintendoDsTextureCodec.Word(ambiguous, entries + 2);
        ambiguous[paletteName] = (byte)'x';
        Assert.Throws<InvalidDataException>(() => new NintendoDsNsbtxTextureImporter().ImportNativeTexture(new MemoryStream(ambiguous)));
        using TextureAsset selected = new NintendoDsNsbtxTextureImporter().ImportNativeTexture(new MemoryStream(ambiguous), 0, 0);
        Assert.Equal(File.ReadAllBytes(Path.Combine(Root(), "a3i5.rgba")), NintendoDsTextureCodec.Decode(selected));
    }
    /// <summary>Checks strict file, section, dictionary and compression/palette bounds before native decode.</summary>
    [Fact]
    public void Corrupted_container_fields_are_rejected() {
        byte[] data = File.ReadAllBytes(Path.Combine(Root(), "a3i5.nsbtx"));
        foreach (int offset in new[] { 0, 5, 6, 8, 12, 14, 16, 20, 24, 34, 40, 80, 82, 86 }) {
            byte old = data[offset]; data[offset] = 255;
            Assert.ThrowsAny<Exception>(() => new NintendoDsNsbtxTextureImporter().ImportTexture(new MemoryStream(data)));
            data[offset] = old;
        }
        Assert.ThrowsAny<Exception>(() => new NintendoDsNsbtxTextureImporter().ImportTexture(new MemoryStream(data.Take(data.Length - 1).ToArray())));
    }
    /// <summary>Runs real builder image and generated font-atlas cooking on an independently authored NSBTX.</summary>
    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    [SupportedOSPlatform("windows")]
    public void Source_processor_imports_native_archives(bool fontAtlas) {
        var processor = new NintendoDsPlatformCookSourceProcessor();
        var settings = new TextureAssetProcessorSettings { ColorFormatId = "NintendoDs.A5I3", AlphaPrecision = TextureAssetAlphaPrecision.A5, MaxResolution = 8 };
        string path = Path.Combine(Root(), "a3i5.nsbtx");
        using TextureAsset cooked = fontAtlas ? processor.CookFontAtlasTexture(path, "font", settings) : processor.CookTexture(path, "texture", settings);
        Assert.Equal(fontAtlas ? "font#atlas" : "texture", cooked.Id);
        Assert.Equal(6, NintendoDsTextureCodec.ReadLayout(cooked).Format.Code);
    }
    /// <summary>Resolves checked-in fixtures without requiring external tooling during ordinary tests.</summary>
    static string Root() => Path.Combine(AppContext.BaseDirectory, "Fixtures", "nsbtx");
}
