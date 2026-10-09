using helengine.editor;

namespace helengine.ds.builder.tests;

/// <summary>Checks all native DS texture/tile encodings, alpha precision, storage bounds and serialization.</summary>
public sealed class NintendoDsNativeTextureTests {
    /// <summary>Enumerates intrinsic and opaque cooking for every published native format.</summary>
    public static IEnumerable<object[]> Formats() {
        foreach (NintendoDsTextureFormat format in NintendoDsTextureFormatCatalog.Formats) {
            yield return new object[] { format.Id, format.Alpha }; yield return new object[] { format.Id, TextureAssetAlphaPrecision.Opaque };
        }
    }
    /// <summary>Exercises native cooking, strict layout checks and both packaged texture readers.</summary>
    [Theory]
    [MemberData(nameof(Formats))]
    public void Native_formats_cook_preview_and_roundtrip(string id, TextureAssetAlphaPrecision alpha) {
        using TextureAsset source = Source(13, 9);
        TextureAssetProcessorSettings settings = new TextureAssetProcessorSettings { ColorFormatId = id, AlphaPrecision = alpha, MaxResolution = 1024 };
        Assert.Equal(id, NintendoDsTextureCookSettingsSerializer.Deserialize(NintendoDsTextureCookSettingsSerializer.Serialize(settings)).ColorFormatId);
        using TextureAsset native = new TextureAssetProcessor().Apply(source, settings);
        NintendoDsTextureLayout layout = NintendoDsTextureCodec.ReadLayout(native);
        Assert.Equal(16, layout.StorageWidth); Assert.Equal(16, layout.StorageHeight);
        Assert.Equal(source.Id, native.Id); Assert.Equal(source.RuntimeAssetId, native.RuntimeAssetId);
        byte[] pixels = TextureAssetPixelCodec.DecodeToRgba32(native);
        Assert.Equal(13 * 9 * 4, pixels.Length);
        if (alpha == TextureAssetAlphaPrecision.Opaque) for (int index = 3; index < pixels.Length; index += 4) Assert.Equal(255, pixels[index]);
        Assert.True(TextureVramUsageCalculator.TryCalculateBytes(13, 9, new helengine.editor.TextureAssetProcessorSettings { ColorFormatId = id, AlphaPrecision = alpha }, out long budget));
        Assert.True(budget >= layout.TexelBytes + layout.DescriptorBytes + native.PaletteColors.Length);
        using MemoryStream stream = new MemoryStream(helengine.files.AssetSerializer.SerializeToBytes(native));
        using TextureAsset file = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.Deserialize(stream));
        Assert.Equal(pixels, NintendoDsTextureCodec.Decode(file));
        stream.Position = 0;
        using TextureAsset runtime = Assert.IsType<TextureAsset>(PackagedAssetBinarySerializer.Deserialize(stream));
        Assert.Equal(native.Colors, runtime.Colors); Assert.Equal(native.PaletteColors, runtime.PaletteColors);
        Assert.Equal(pixels, NintendoDsTextureCodec.Decode(runtime));
        foreach (int offset in new[] { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44 }) {
            byte old = native.Colors[offset]; native.Colors[offset] = 255;
            Assert.ThrowsAny<ArgumentException>(() => NintendoDsTextureCodec.ReadLayout(native)); native.Colors[offset] = old;
        }
        if (alpha == layout.Format.Alpha) {
            string prefix = Path.Combine(OutputRoot(), id);
            File.WriteAllBytes(prefix + ".dst", native.Colors); File.WriteAllBytes(prefix + ".pal", native.PaletteColors); File.WriteAllBytes(prefix + ".rgba", pixels);
        }
    }
    /// <summary>Checks red-low RGB5551 words, packed low-first indices and tile boundary addresses independently.</summary>
    [Fact]
    public void Native_words_indices_and_tiles_have_literal_layouts() {
        using TextureAsset source = new TextureAsset { Width = 2, Height = 1, ColorFormat = TextureAssetColorFormat.Rgba32, AlphaPrecision = TextureAssetAlphaPrecision.A8,
            Colors = new byte[] { 255, 0, 0, 255, 0, 255, 0, 0 } };
        using TextureAsset direct = NintendoDsTextureCodec.Encode(source, "NintendoDs.Bgr5551", TextureAssetAlphaPrecision.Binary);
        Assert.Equal(new byte[] { 31, 128, 224, 3 }, direct.Colors.Skip(48).Take(4));
        Assert.Equal(8, NintendoDsTextureLayout.PixelIndex(8, 0, 16, false));
        Assert.Equal(64, NintendoDsTextureLayout.PixelIndex(8, 0, 16, true));
        Assert.Equal(8, NintendoDsTextureLayout.PixelIndex(0, 1, 16, true));
        Assert.Equal(128, NintendoDsTextureLayout.PixelIndex(0, 8, 16, true));
        using TextureAsset indexed = NintendoDsTextureCodec.CreateNative(8, 8, 2, TextureAssetAlphaPrecision.Binary, true,
            Enumerable.Repeat((byte)0xe4, 16).ToArray(), new byte[] { 0, 0, 31, 0, 224, 3, 0, 124 });
        Assert.Equal(new byte[] { 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255 }, NintendoDsTextureCodec.Decode(indexed).Take(16));
    }
    /// <summary>Checks the native A3-to-A5 expansion and all thirty-two A5 alpha values against literal hardware levels.</summary>
    [Theory]
    [InlineData(1)]
    [InlineData(6)]
    public void Native_alpha_uses_ds_five_bit_expansion(int code) {
        byte[] texels = new byte[64];
        for (int pixel = 0; pixel < 64; pixel++) texels[pixel] = (byte)((pixel % (code == 1 ? 8 : 32)) << (code == 1 ? 5 : 3));
        using TextureAsset texture = NintendoDsTextureCodec.CreateNative(8, 8, code, NintendoDsTextureFormatCatalog.GetFormat(code).Alpha, false, texels, new byte[] { 255, 127 });
        byte[] rgba = NintendoDsTextureCodec.Decode(texture);
        for (int pixel = 0; pixel < 64; pixel++) {
            int a = pixel % (code == 1 ? 8 : 32);
            int five = code == 1 ? (a << 2) | (a >> 1) : a;
            Assert.Equal((five << 3) | (five >> 2), rgba[pixel * 4 + 3]);
        }
        if (code == 1) Assert.Equal(new byte[] { 0, 33, 74, 107, 148, 181, 222, 255 }, Enumerable.Range(0, 8).Select(index => rgba[index * 4 + 3]));
    }
    /// <summary>Rejects GPU over-allocation, unsupported alpha and references beyond native palettes.</summary>
    [Fact]
    public void Native_limits_and_palette_references_are_checked() {
        Assert.Throws<ArgumentException>(() => new NintendoDsTextureLayout(NintendoDsTextureFormatCatalog.GetFormat(5), 1024, 1024));
        Assert.Throws<ArgumentException>(() => new NintendoDsTextureLayout(NintendoDsTextureFormatCatalog.GetFormat(7), 1024, 1024));
        Assert.Throws<ArgumentOutOfRangeException>(() => new NintendoDsTextureLayout(NintendoDsTextureFormatCatalog.GetFormat(3), 1025, 1));
        Assert.Throws<ArgumentException>(() => NintendoDsNativeTextureCooker.ValidateSettings(new TextureAssetProcessorSettings { ColorFormatId = "NintendoDs.A5I3", AlphaPrecision = TextureAssetAlphaPrecision.A8 }));
        Assert.Throws<ArgumentException>(() => NintendoDsTextureCodec.CreateNative(8, 8, 2, TextureAssetAlphaPrecision.Binary, false, Enumerable.Repeat((byte)255, 16).ToArray(), new byte[] { 0, 0 }));
        byte[] compressed = new byte[24]; compressed[16] = 255; compressed[17] = 255;
        Assert.Throws<ArgumentException>(() => NintendoDsTextureCodec.CreateNative(8, 8, 5, TextureAssetAlphaPrecision.Binary, false, compressed, new byte[8]));
    }
    /// <summary>Checks exact published formats, native alpha combinations and legacy defaults.</summary>
    [Fact]
    public void Capabilities_publish_native_formats_and_keep_legacy_defaults() {
        var definition = NintendoDsPlatformDefinitionFactory.Create();
        foreach (var capability in definition.AssetCookCapabilities.Where(item => item.SourceAssetKind == "texture" || item.SourceAssetKind == "font-atlas-texture")) {
            Assert.Equal(12, capability.TextureFormatCapabilities.SupportedColorFormatIds.Length);
            foreach (NintendoDsTextureFormat format in NintendoDsTextureFormatCatalog.Formats) Assert.Contains(format.Id, capability.TextureFormatCapabilities.SupportedColorFormatIds);
            Assert.Contains("Rgba4444", capability.TextureFormatCapabilities.SupportedColorFormatIds);
        }
    }
    /// <summary>Builds an authored non-power-of-two pattern containing opaque and transparent colors.</summary>
    public static TextureAsset Source(int width, int height) {
        byte[] rgba = new byte[width * height * 4];
        for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4; rgba[index] = (byte)(x * 19); rgba[index + 1] = (byte)(y * 29); rgba[index + 2] = (byte)((x + y) * 17); rgba[index + 3] = (byte)((x + y) * 23);
        }
        return new TextureAsset { Id = "ds-texture", RuntimeAssetId = 42, Width = (ushort)width, Height = (ushort)height, ColorFormat = TextureAssetColorFormat.Rgba32, AlphaPrecision = TextureAssetAlphaPrecision.A8, Colors = rgba };
    }
    /// <summary>Uses visible workspace-owned fixtures for agent runs or local build outputs for ordinary test runs.</summary>
    static string OutputRoot() {
        string path = Environment.GetEnvironmentVariable("HELENGINE_DS_TEXTURE_TEST_OUTPUT") ?? Path.Combine(AppContext.BaseDirectory, "native-fixtures");
        Directory.CreateDirectory(path); return path;
    }
}
