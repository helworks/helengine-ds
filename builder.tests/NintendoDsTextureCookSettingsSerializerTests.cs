namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies Nintendo DS texture cook settings serialize using the current generic builder contract.
/// </summary>
public sealed class NintendoDsTextureCookSettingsSerializerTests {
    /// <summary>
    /// Ensures serializing one settings object preserves the configured color-format identifier without recursion.
    /// </summary>
    [Fact]
    public void Serialize_whenUsingSettingsObject_preservesColorFormatId() {
        TextureAssetProcessorSettings settings = new TextureAssetProcessorSettings {
            MaxResolution = 128,
            ColorFormatId = TextureAssetColorFormat.Indexed8.ToString(),
            AlphaPrecision = TextureAssetAlphaPrecision.A8
        };

        string serializedSettings = NintendoDsTextureCookSettingsSerializer.Serialize(settings);
        TextureAssetProcessorSettings deserializedSettings = NintendoDsTextureCookSettingsSerializer.Deserialize(serializedSettings);

        Assert.Equal(128, deserializedSettings.MaxResolution);
        Assert.Equal(TextureAssetColorFormat.Indexed8.ToString(), deserializedSettings.ColorFormatId);
        Assert.Equal(TextureAssetAlphaPrecision.A8, deserializedSettings.AlphaPrecision);
    }
}
