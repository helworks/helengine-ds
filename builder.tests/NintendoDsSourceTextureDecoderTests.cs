using System.Runtime.Versioning;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies Nintendo DS source texture decoding for both bitmap files and serialized texture assets.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
[SupportedOSPlatform("windows")]
public sealed class NintendoDsSourceTextureDecoderTests {
    /// <summary>
    /// Verifies the decoder accepts one serialized texture asset even when the file extension is not <c>.hasset</c>.
    /// </summary>
    [Fact]
    public void Decode_when_source_contains_serialized_texture_asset_without_hasset_extension_returns_texture_asset() {
        string temporaryDirectoryPath = Path.Combine(Path.GetTempPath(), "helengine-ds-source-texture-decoder-tests", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(temporaryDirectoryPath);
        string sourceAssetPath = Path.Combine(temporaryDirectoryPath, "generated-text-sprite.bin");
        TextureAsset expectedTextureAsset = new() {
            Id = "generated:text-sprite:test",
            Width = 2,
            Height = 1,
            Colors = [1, 2, 3, 4, 5, 6, 7, 8],
            PaletteColors = Array.Empty<byte>(),
            ColorFormat = TextureAssetColorFormat.Rgba32,
            AlphaPrecision = TextureAssetAlphaPrecision.A8
        };

        try {
            File.WriteAllBytes(sourceAssetPath, global::helengine.files.AssetSerializer.SerializeToBytes(expectedTextureAsset));

            NintendoDsSourceTextureDecoder decoder = new();
            TextureAsset decodedTextureAsset = decoder.Decode(sourceAssetPath);

            Assert.Equal(expectedTextureAsset.Id, decodedTextureAsset.Id);
            Assert.Equal(expectedTextureAsset.Width, decodedTextureAsset.Width);
            Assert.Equal(expectedTextureAsset.Height, decodedTextureAsset.Height);
            Assert.Equal(expectedTextureAsset.ColorFormat, decodedTextureAsset.ColorFormat);
            Assert.Equal(expectedTextureAsset.AlphaPrecision, decodedTextureAsset.AlphaPrecision);
            Assert.Equal(expectedTextureAsset.Colors, decodedTextureAsset.Colors);
        } finally {
            if (Directory.Exists(temporaryDirectoryPath)) {
                Directory.Delete(temporaryDirectoryPath, recursive: true);
            }
        }
    }
}
